#!/usr/bin/env python3
"""TDMS compatibility of tdms_lib with npTDMS (https://github.com/adamreeve/npTDMS), an independent
reader and writer of the format.

  1. tdms_compat_tool writes the reference file with SegmentBuilder, npTDMS reads it.
  2. npTDMS writes files, FileReader reads them (tdms_compat_tool dump), and the two readings agree.

Usage: compat_nptdms.py <tdms_compat_tool>. Exits 77 (skipped) when numpy or npTDMS is missing:
pip install npTDMS.
"""
import json
import os
import subprocess
import sys
import tempfile

try:
    import numpy as np
    import nptdms.tdms_segment
    from nptdms import ChannelObject, GroupObject, RootObject, TdmsFile, TdmsWriter, types
    from nptdms.timestamp import TdmsTimestamp
except ImportError as e:
    print(f"skipped: {e}; pip install npTDMS")
    sys.exit(77)

# npTDMS gives property values as Python values; this keeps the TDMS type of each property name
# as it reads them (the names in these files have one type each)
property_types = {}


def read_property(f, endianness="<"):
    name = types.String.read(f, endianness)
    code = types.Uint32.read(f, endianness)
    property_types[name] = code
    return name, types.tds_data_types[code].read(f, endianness)


nptdms.tdms_segment.read_property = read_property

TOOL = sys.argv[1]
U64_MAX = 18446744073709551615
EPOCH_1904 = 2082844800
failures = []


def check(ok, what):
    if not ok:
        failures.append(what)
        print("FAIL", what)


def unix_ns(t):
    """A TdmsTimestamp or numpy datetime64 as nanoseconds since 1970."""
    if isinstance(t, TdmsTimestamp):
        return (t.seconds - EPOCH_1904) * 1000000000 + round(t.second_fractions * 1000000000 / 2 ** 64)
    return int(np.datetime64(t, "ns").astype(np.int64))


def plain(v):
    if isinstance(v, (TdmsTimestamp, np.datetime64)):
        return unix_ns(v)
    if isinstance(v, np.generic):
        return v.item()
    return v


# ---- 1. SegmentBuilder -> npTDMS --------------------------------------------------------------
def check_reference(path):
    f = TdmsFile.read(path, raw_timestamps=True)
    p = f.properties
    expected = {"name": "reference", "i8": -8, "i16": -16, "i32": -32, "i64": -64, "u8": 8, "u16": 16, "u32": 32,
                "u64": U64_MAX, "f32": 0.5, "f64": -2.25, "flag": True, "text": "line\nwith \"quotes\" and π"}
    expected_types = {"i8": 1, "i16": 2, "i32": 3, "i64": 4, "u8": 5, "u16": 6, "u32": 7, "u64": 8, "f32": 9, "f64": 10,
             "flag": 0x21, "text": 0x20, "name": 0x20, "start": 0x44}
    for k, v in expected.items():
        check(k in p and plain(p[k]) == v, f"reference: file property {k} = {p.get(k)!r}, expected {v!r}")
    check(unix_ns(p["start"]) == 1700000000123456789, f"reference: start {p['start']!r}")
    # the fraction of a second is the nearest 2^-64 s
    check(p["start"].second_fractions == (123456789 * 2 ** 64 + 500000000) // 1000000000, f"reference: start fraction {p['start']!r}")
    for k, t in expected_types.items():
        check(property_types.get(k) == t, f"reference: file property {k} has type {property_types.get(k)}, expected {t}")

    g = f["Group"]
    check(plain(g.properties["decimation"]) == 8 and property_types.get("decimation") == 7, "reference: decimation")
    check(plain(g.properties["segment"]) == 2, f"reference: the last segment property wins, got {g.properties['segment']}")

    ch1 = g["ch1"]
    check(ch1.data_type.enum_value == 2, "reference: ch1 is Integer16")
    check(list(ch1[:]) == [s * 100 + i - 2 for s in range(3) for i in range(4)], f"reference: ch1 data {list(ch1[:])}")
    check(plain(ch1.properties["wf_increment"]) == 64e-9 and ch1.properties["unit_string"] == "V", "reference: ch1 properties")
    # the waveform properties give npTDMS the time of every sample
    track = ch1.time_track()
    check(len(track) == 12 and abs(track[0] - 0.5) < 1e-12 and abs(track[11] - (0.5 + 11 * 64e-9)) < 1e-12,
          f"reference: ch1 time track {track[:2]}...{track[-1:]}")
    check(unix_ns(ch1.properties["wf_start_time"]) == 1700000000 * 1000000000, "reference: ch1 wf_start_time")
    ch2 = g["ch2"]
    check(ch2.data_type.enum_value == 9, "reference: ch2 is SingleFloat")
    check(list(ch2[:]) == [s + i * 0.25 for s in (0, 2) for i in range(4)], f"reference: ch2 has no data in segment 1: {list(ch2[:])}")

    q = f["It's"]["a'b"]
    check(q.data_type.enum_value == 8 and [int(x) for x in q[:]] == [0, U64_MAX, 1, U64_MAX, 2, U64_MAX],
          f"reference: names with quotes, uint64 data {list(q[:])}")
    check(q.properties["quoted"] is True or q.properties["quoted"] == np.True_, "reference: quoted property")


# ---- 2. npTDMS -> FileReader ------------------------------------------------------------------
def write_nptdms(path, version):
    props = {"name": "nptdms", "i8": np.int8(-8), "i16": np.int16(-16), "i32": np.int32(-32), "i64": np.int64(-64),
             "u8": np.uint8(8), "u16": np.uint16(16), "u32": np.uint32(32), "u64": np.uint64(U64_MAX),
             "f32": np.float32(0.25), "f64": np.float64(1e-300), "flag": False,
             "start": np.datetime64("2023-11-14T22:13:20.123456789", "ns"), "text": "it's \"text\"\nπ"}
    data = {"i8": np.array([-128, 0, 127], np.int8), "i16": np.array([-32768, 1, 32767], np.int16),
            "i32": np.array([-1, 2, 3], np.int32), "i64": np.array([-(2 ** 63), 0, 2 ** 63 - 1], np.int64),
            "u8": np.array([0, 1, 255], np.uint8), "u16": np.array([0, 2, 65535], np.uint16),
            "u32": np.array([0, 3, 2 ** 32 - 1], np.uint32), "u64": np.array([0, 4, U64_MAX], np.uint64),
            "f32": np.array([0.5, -1.5, 3.25], np.float32), "f64": np.array([1e300, -0.0, 2.5], np.float64)}
    with TdmsWriter(path, version=version) as w:
        for seg in range(3):
            objects = []
            group = {"segment": np.int32(seg)}
            if seg == 0:
                objects.append(RootObject(properties=props))
                objects.append(ChannelObject("Info", "only properties", [], properties={"note": "no samples"}))
                group["decimation"] = np.uint32(8)
            objects.append(GroupObject("Data", properties=group))
            for name, values in data.items():
                if seg == 1 and name == "f64":
                    continue  # a channel missing from a segment
                objects.append(ChannelObject("Data", name, values, properties={"wf_increment": 1e-6} if seg == 0 else {}))
            w.write_segment(objects)


def compare(path, label):
    out = subprocess.run([TOOL, "dump", path], capture_output=True, text=True)
    if out.returncode != 0:
        check(False, f"{label}: FileReader refused the file: {out.stderr.strip()}")
        return
    ours = json.loads(out.stdout)
    f = TdmsFile.read(path, raw_timestamps=True)

    def same_properties(mine, theirs, where):
        check(set(mine) == set(theirs), f"{label}: {where} property names {sorted(mine)} vs {sorted(theirs)}")
        for k, v in theirs.items():
            if k not in mine:
                continue
            check(mine[k]["value"] == plain(v), f"{label}: {where} property {k} = {mine[k]['value']!r}, npTDMS {plain(v)!r}")
            t = property_types.get(k)
            check(mine[k]["type"] == t, f"{label}: {where} property {k} type {mine[k]['type']}, npTDMS {t}")

    same_properties(ours["properties"], f.properties, "file")
    for g in f.groups():
        check(g.name in ours["groups"], f"{label}: group {g.name} missing")
        same_properties(ours["groups"].get(g.name, {}), g.properties, f"group {g.name}")
    mine = {(c["group"], c["name"]): c for c in ours["channels"]}
    theirs = [(g.name, c) for g in f.groups() for c in g.channels()]
    check(len(mine) == len(theirs), f"{label}: {len(mine)} channels, npTDMS {len(theirs)}")
    for group, c in theirs:
        m = mine.get((group, c.name))
        if m is None:
            check(False, f"{label}: channel {group}/{c.name} missing")
            continue
        same_properties(m["properties"], c.properties, f"channel {group}/{c.name}")
        values = [v.item() for v in c[:]]
        check(m["count"] == len(values), f"{label}: {group}/{c.name} count {m['count']}, npTDMS {len(values)}")
        if values:
            check(m["type"] == c.data_type.enum_value, f"{label}: {group}/{c.name} type {m['type']}, npTDMS {c.data_type.enum_value}")
            check(m["data"] == values, f"{label}: {group}/{c.name} data {m['data']} vs {values}")


def run(step, label, *args):
    try:
        step(*args)
    except Exception as e:  # a file one side cannot make sense of is a failure, not a crash of the test
        check(False, f"{label}: {type(e).__name__}: {e}")


with tempfile.TemporaryDirectory() as d:
    reference = os.path.join(d, "reference.tdms")
    if subprocess.run([TOOL, "write", reference]).returncode != 0:
        check(False, "tdms_compat_tool could not write the reference file")
    else:
        run(check_reference, "reference, npTDMS", reference)
        run(compare, "reference, FileReader", reference, "reference")
    for version in (4712, 4713):
        path = os.path.join(d, f"nptdms_{version}.tdms")
        run(write_nptdms, f"npTDMS {version}, writing", path, version)
        run(compare, f"npTDMS {version}", path, f"npTDMS {version}")

print(f"npTDMS compatibility: {'FAILED, ' + str(len(failures)) + ' problems' if failures else 'OK'}")
sys.exit(1 if failures else 0)
