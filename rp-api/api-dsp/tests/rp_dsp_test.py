#!/usr/bin/python3

import rp_dsp
import math
import sys
import random

# ============================================================================
# Helper Functions
# ============================================================================

def compare_float(actual, expected, tolerance=1e-6):
    """Compare two floats with tolerance"""
    if abs(expected) < 1e-10:
        return abs(actual) < tolerance
    return abs(actual - expected) / abs(expected) < tolerance

def compare_double(actual, expected, tolerance=1e-10):
    """Compare two doubles with tighter tolerance"""
    if abs(expected) < 1e-15:
        return abs(actual) < tolerance
    return abs(actual - expected) / abs(expected) < tolerance

def test_result(name, passed):
    """Print test result"""
    status = "PASS" if passed else "FAIL"
    print(f"  [{status}] {name}")

def test_section_header(title):
    """Print section header"""
    print("\n" + "=" * 80)
    print(f"SECTION: {title}")
    print("=" * 80)

# ============================================================================
# SECTION 1: DSP Object Basic Tests
# ============================================================================

test_section_header("DSP Object Basic Tests")

print("\n--- Creating DSP Object ---")
print("rp_dsp.CDSP(2, 256, 125000000)")
obj = rp_dsp.CDSP(2, 256, 125000000)
test_result("Create DSP object", obj is not None)

print("\n--- Channel Configuration ---")
print("obj.setChannel(0, True)")
obj.setChannel(0, True)
test_result("Set Channel 0", True)

print("obj.setChannel(1, True)")
obj.setChannel(1, True)
test_result("Set Channel 1", True)

print("obj.getChannel(0, ...)")
ch0_enabled = obj.getChannel(0)
test_result(f"Get Channel 0 = {ch0_enabled}", ch0_enabled == True)

print("obj.getChannel(1, ...)")
ch1_enabled = obj.getChannel(1)
test_result(f"Get Channel 1 = {ch1_enabled}", ch1_enabled == True)

print("\n--- Signal Length Configuration ---")
print("obj.setSignalLength(256)")
res = obj.setSignalLength(256)
test_result("Set Signal Length", res == 0)

print("obj.getSignalLength()")
res = obj.getSignalLength()
test_result(f"Get Signal Length = {res}", res == 256)

print("obj.getSignalMaxLength()")
res = obj.getSignalMaxLength()
print(f"  Max Signal Length: {res}")

print("obj.getOutSignalLength()")
res = obj.getOutSignalLength()
print(f"  Out Signal Length: {res}")

print("obj.getOutSignalMaxLength()")
out_signal = obj.getOutSignalMaxLength()
print(f"  Out Signal Max Length: {out_signal}")

print("\n--- Window Configuration ---")
# Test all window modes
window_modes = [
    (rp_dsp.RECTANGULAR, "RECTANGULAR"),
    (rp_dsp.HANNING, "HANNING"),
    (rp_dsp.HAMMING, "HAMMING"),
    (rp_dsp.BLACKMAN_HARRIS, "BLACKMAN_HARRIS"),
    (rp_dsp.FLAT_TOP, "FLAT_TOP"),
    (rp_dsp.KAISER_4, "KAISER_4"),
    (rp_dsp.KAISER_8, "KAISER_8"),
]

print("  Testing all window modes:")
for mode, name in window_modes:
    obj.window_init(mode)
    current = obj.getCurrentWindowMode()
    test_result(f"  Window {name}", current == mode)

print("\n--- Impedance Configuration ---")
print("obj.setImpedance(100.0)")
obj.setImpedance(100.0)
test_result("Set Impedance", True)

print("obj.getImpedance()")
res = obj.getImpedance()
test_result(f"Get Impedance = {res}", abs(res - 100.0) < 1e-10)

print("\n--- DC Removal Configuration ---")
print("obj.setRemoveDC(True)")
obj.setRemoveDC(True)
test_result("Set Remove DC", True)

print("obj.getRemoveDC()")
res = obj.getRemoveDC()
test_result(f"Get Remove DC = {res}", res == True)

# Test with False
print("obj.setRemoveDC(False)")
obj.setRemoveDC(False)
res = obj.getRemoveDC()
test_result(f"Get Remove DC = {res} after set False", res == False)

# Restore
obj.setRemoveDC(True)

print("\n--- Probe Configuration ---")
print("obj.setProbe(0, 10)")
obj.setProbe(0, 10)
test_result("Set Probe Ch0", True)

print("obj.setProbe(1, 100)")
obj.setProbe(1, 100)
test_result("Set Probe Ch1", True)

probe0 = obj.getProbe(0)
test_result(f"Get Probe Ch0 = {probe0}", probe0 == 10)

probe1 = obj.getProbe(1)
test_result(f"Get Probe Ch1 = {probe1}", probe1 == 100)

# ============================================================================
# SECTION 2: FFT and Signal Processing Tests
# ============================================================================

test_section_header("FFT and Signal Processing Tests")

print("\n--- FFT Initialization ---")
print("obj.fftInit()")
res = obj.fftInit()
test_result("FFT Init", res == 0)

print("\n--- Data Creation ---")
print("obj.createData()")
data = obj.createData()
test_result("Create Data", data is not None)

# Test data structure
print("\n  Data structure tests:")
test_result("  data.m_in exists", hasattr(data, 'm_in'))
test_result("  data.m_filtred exists", hasattr(data, 'm_filtred'))
test_result("  data.m_fft exists", hasattr(data, 'm_fft'))
test_result("  data.m_dec_data_z exists", hasattr(data, 'm_dec_data_z'))
test_result("  data.m_dec_data_wsumf exists", hasattr(data, 'm_dec_data_wsumf'))
test_result("  data.m_dec_data_scaled exists", hasattr(data, 'm_dec_data_scaled'))
test_result("  data.m_converted exists", hasattr(data, 'm_converted'))
test_result("  data.m_is_data_filtred exists", hasattr(data, 'm_is_data_filtred'))

# Test reset
print("\n  Testing data.reset():")
data.m_is_data_filtred = True
data.reset()
test_result("  data.reset()", data.m_is_data_filtred == False)

# Access data through the extend accessor methods
print("\n--- Setting Test Data ---")

# Get channel data using the accessor (returns std::vector<float>&)
print("data.getChannelData(0)")
data_in_ch1 = data.getChannelData(0)
test_result("  Get channel 0 data", data_in_ch1 is not None)
print(f"  Channel 0 size: {len(data_in_ch1)}")

print("data.getChannelData(1)")
data_in_ch2 = data.getChannelData(1)
test_result("  Get channel 1 data", data_in_ch2 is not None)
print(f"  Channel 1 size: {len(data_in_ch2)}")

# Set test values directly (getChannelData returns a reference)
data_in_ch1[0] = 0.0
data_in_ch1[1] = 1.0
data_in_ch1[2] = 2.0
data_in_ch1[3] = 3.0

print(data_in_ch1[0])
print(data_in_ch1[1])
print(data_in_ch1[2])
print(data_in_ch1[3])

# Verify test values
all_data_ok = True
if data_in_ch1[0] != 0.0: all_data_ok = False
if data_in_ch1[1] != 1.0: all_data_ok = False
if data_in_ch1[2] != 2.0: all_data_ok = False
if data_in_ch1[3] != 3.0: all_data_ok = False
test_result("Set and verify test data", all_data_ok)


# Test reset via accessor
data.setDataFiltered(True)
data.reset()
test_result("  data.reset()", not data.isDataFiltered())

print("\n--- Signal Processing Pipeline ---")
print("obj.prepareFreqVector(data, 125000000.0, 1.0)")
res = obj.prepareFreqVector(data, 125000000.0, 1.0)
test_result("Prepare Freq Vector (with adc_rate)", res == 0)

print("obj.windowFilter(data)")
res = obj.windowFilter(data)
test_result("Window Filter", res == 0)
test_result("  m_is_data_filtred after windowFilter", data.m_is_data_filtred == True)

print("obj.fft(data)")
res = obj.fft(data)
test_result("FFT", res == 0)

print("obj.decimate(data, 256, 128)")
res = obj.decimate(data, 256, 128)
test_result("Decimate", res == 0)

print("obj.cnvToMetric(data, 1, 0, 100000)")
res = obj.cnvToMetric(data, 1, 0, 100000)
test_result("Convert to Metric", res == 0)

# Test getAmpAndPhase with doublePtr
print("\n  Testing getAmpAndPhase:")
print("obj.getAmpAndPhase(data, 1000.0)")
res,amp1, phase1, amp2, phase2 = obj.getAmpAndPhase(data, 1000.0)
test_result("getAmpAndPhase", res == 0)
print(f"    amp1={amp1:.6f}, phase1={phase1:.6f}")
print(f"    amp2={amp2:.6f}, phase2={phase2:.6f}")

print("\nobj.remoteDCCount()")
res = obj.remoteDCCount()
print(f"  DC Count: {res}")

# ============================================================================
# SECTION 3: Basic Math Function Tests
# ============================================================================

test_section_header("Basic Math Function Tests (log10f_neon, sqrtf_neon)")

print("\n--- log10f_neon Tests ---")
test_values = [0.001, 0.01, 0.1, 1.0, 10.0, 100.0, 1000.0, 10000.0]
all_passed = True
max_error = 0.0

for val in test_values:
    neon_result = rp_dsp.log10f_neon(val)
    expected = math.log10(val)
    error = abs(neon_result - expected)
    if error > max_error:
        max_error = error
    passed = error < 1e-5
    if not passed:
        all_passed = False
    print(f"  log10f_neon({val:10.4f}) = {neon_result:12.8f} (expected: {expected:12.8f}) error: {error:.2e} {'✓' if passed else '✗'}")

test_result(f"log10f_neon all tests (max error: {max_error:.2e})", all_passed)

# Edge cases for log10
print("\n  Edge cases:")
try:
    res = rp_dsp.log10f_neon(-1.0)
    print(f"  log10f_neon(-1.0) = {res} (negative input)")
except Exception as e:
    print(f"  log10f_neon(-1.0) raised: {e}")

try:
    res = rp_dsp.log10f_neon(0.0)
    print(f"  log10f_neon(0.0) = {res} (zero input)")
except Exception as e:
    print(f"  log10f_neon(0.0) raised: {e}")

print("\n--- sqrtf_neon Tests ---")
test_values = [0.0, 0.01, 0.25, 1.0, 2.0, 4.0, 9.0, 16.0, 25.0, 49.0, 64.0, 81.0, 100.0, 10000.0]
all_passed = True
max_error = 0.0

for val in test_values:
    neon_result = rp_dsp.sqrtf_neon(val)
    expected = math.sqrt(val)
    error = abs(neon_result - expected)
    if error > max_error:
        max_error = error
    passed = error < 1e-5
    if not passed:
        all_passed = False
    print(f"  sqrtf_neon({val:10.4f}) = {neon_result:12.8f} (expected: {expected:12.8f}) error: {error:.2e} {'✓' if passed else '✗'}")

test_result(f"sqrtf_neon all tests (max error: {max_error:.2e})", all_passed)

# Edge case for sqrt
print("\n  Edge cases:")
try:
    res = rp_dsp.sqrtf_neon(-1.0)
    print(f"  sqrtf_neon(-1.0) = {res} (negative input)")
except Exception as e:
    print(f"  sqrtf_neon(-1.0) raised: {e}")

# ============================================================================
# SECTION 4: Array Arithmetic Tests - Float
# ============================================================================

test_section_header("Array Arithmetic Tests - Float (float*)")

SIZE = 256

def test_float_array_operation(name, func, scalar_op, size=SIZE):
    """Test float array operations"""
    print(f"\n--- {name} ---")

    # Create test arrays
    src1 = rp_dsp.arrFloat(size)
    src2 = rp_dsp.arrFloat(size)
    dst = rp_dsp.arrFloat(size)

    # Generate test data
    test_data = []
    for i in range(size):
        v1 = random.uniform(-100.0, 100.0)
        v2 = random.uniform(-100.0, 100.0)
        src1[i] = v1
        src2[i] = v2
        test_data.append((v1, v2))

    # Call function under test
    func(dst.cast(), src1.cast(), src2.cast(), size)

    # Verify results
    all_passed = True
    max_error = 0.0
    errors_found = 0

    for i in range(size):
        v1, v2 = test_data[i]
        expected_val = scalar_op(v1, v2)
        actual_val = dst[i]
        error = abs(actual_val - expected_val)
        if error > max_error:
            max_error = error
        if not compare_float(actual_val, expected_val, 1e-5):
            all_passed = False
            errors_found += 1
            if errors_found <= 3:
                print(f"  [{i}] {v1:.4f} op {v2:.4f} = {actual_val:.6f} (expected: {expected_val:.6f}) error: {error:.2e}")

    if errors_found > 3:
        print(f"  ... and {errors_found - 3} more errors")

    test_result(f"{name} (errors: {errors_found}, max error: {max_error:.2e})", all_passed)
    return all_passed

# Test float add
try:
    test_float_array_operation("add_arrays_float_neon",
                              rp_dsp.add_arrays_float_neon,
                              lambda a, b: a + b)
except AttributeError as e:
    print(f"  [SKIP] add_arrays_float_neon not available: {e}")

# Test float subtract
try:
    test_float_array_operation("subtract_arrays_float_neon",
                              rp_dsp.subtract_arrays_float_neon,
                              lambda a, b: a - b)
except AttributeError as e:
    print(f"  [SKIP] subtract_arrays_float_neon not available: {e}")

# Test float multiply
try:
    test_float_array_operation("multiply_arrays_float_neon",
                              rp_dsp.multiply_arrays_float_neon,
                              lambda a, b: a * b)
except AttributeError as e:
    print(f"  [SKIP] multiply_arrays_float_neon not available: {e}")

# Test float divide (with zero avoidance)
try:
    print(f"\n--- divide_arrays_float_neon ---")
    src1 = rp_dsp.arrFloat(SIZE)
    src2 = rp_dsp.arrFloat(SIZE)
    dst = rp_dsp.arrFloat(SIZE)

    test_data = []
    for i in range(SIZE):
        v1 = random.uniform(-100.0, 100.0)
        v2 = random.uniform(-100.0, 100.0)
        if abs(v2) < 0.001:
            v2 = 1.0
        src1[i] = v1
        src2[i] = v2
        test_data.append((v1, v2))

    rp_dsp.divide_arrays_float_neon(dst.cast(), src1.cast(), src2.cast(), SIZE)

    all_passed = True
    max_error = 0.0
    for i in range(SIZE):
        v1, v2 = test_data[i]
        expected_val = v1 / v2
        actual_val = dst[i]
        error = abs(actual_val - expected_val)
        if error > max_error:
            max_error = error
        if not compare_float(actual_val, expected_val, 1e-3):
            all_passed = False

    test_result(f"divide_arrays_float_neon (max error: {max_error:.2e})", all_passed)
except AttributeError as e:
    print(f"  [SKIP] divide_arrays_float_neon not available: {e}")

# ============================================================================
# SECTION 5: Scalar-Array Operations - Float
# ============================================================================

test_section_header("Scalar-Array Operations - Float")

def test_scalar_float_operation(name, func, scalar_op, scalar_val, size=SIZE):
    """Test scalar-float array operations"""
    print(f"\n--- {name} (scalar={scalar_val}) ---")

    src = rp_dsp.arrFloat(size)
    dst = rp_dsp.arrFloat(size)

    test_data = []
    for i in range(size):
        v = random.uniform(-100.0, 100.0)
        src[i] = v
        test_data.append(v)

    func(dst.cast(), src.cast(), scalar_val, size)

    all_passed = True
    max_error = 0.0
    errors_found = 0

    for i in range(size):
        expected_val = scalar_op(test_data[i], scalar_val)
        actual_val = dst[i]
        error = abs(actual_val - expected_val)
        if error > max_error:
            max_error = error
        if not compare_float(actual_val, expected_val, 1e-5):
            all_passed = False
            errors_found += 1
            if errors_found <= 3:
                print(f"  [{i}] {test_data[i]:.4f} op {scalar_val} = {actual_val:.6f} (expected: {expected_val:.6f})")

    if errors_found > 3:
        print(f"  ... and {errors_found - 3} more errors")

    test_result(f"{name} (errors: {errors_found}, max error: {max_error:.2e})", all_passed)
    return all_passed

# Test float scalar add
try:
    test_scalar_float_operation("add_scalar_to_array_float_neon",
                               rp_dsp.add_scalar_to_array_float_neon,
                               lambda a, s: a + s, 5.0)
except AttributeError as e:
    print(f"  [SKIP] add_scalar_to_array_float_neon not available: {e}")

# Test float scalar subtract
try:
    test_scalar_float_operation("subtract_scalar_from_array_float_neon",
                               rp_dsp.subtract_scalar_from_array_float_neon,
                               lambda a, s: a - s, 3.0)
except AttributeError as e:
    print(f"  [SKIP] subtract_scalar_from_array_float_neon not available: {e}")

# Test float scalar multiply
try:
    test_scalar_float_operation("multiply_array_by_scalar_float_neon",
                               rp_dsp.multiply_array_by_scalar_float_neon,
                               lambda a, s: a * s, 2.5)
except AttributeError as e:
    print(f"  [SKIP] multiply_array_by_scalar_float_neon not available: {e}")

# Test float scalar divide
try:
    test_scalar_float_operation("divide_array_by_scalar_float_neon",
                               rp_dsp.divide_array_by_scalar_float_neon,
                               lambda a, s: a / s if s != 0 else float('inf'), 4.0)
except AttributeError as e:
    print(f"  [SKIP] divide_array_by_scalar_float_neon not available: {e}")

# ============================================================================
# SECTION 6: Array Arithmetic Tests - Integer
# ============================================================================

test_section_header("Array Arithmetic Tests - Integer (int*)")

def test_int_array_operation(name, func, scalar_op, size=SIZE):
    """Test integer array operations"""
    print(f"\n--- {name} ---")

    src1 = rp_dsp.arrInt(size)
    src2 = rp_dsp.arrInt(size)
    dst = rp_dsp.arrInt(size)

    test_data = []
    for i in range(size):
        v1 = random.randint(-1000, 1000)
        v2 = random.randint(-1000, 1000)
        src1[i] = v1
        src2[i] = v2
        test_data.append((v1, v2))

    func(dst.cast(), src1.cast(), src2.cast(), size)

    all_passed = True
    errors_found = 0
    for i in range(size):
        v1, v2 = test_data[i]
        expected_val = scalar_op(v1, v2)
        actual_val = dst[i]
        if actual_val != expected_val:
            all_passed = False
            errors_found += 1
            if errors_found <= 3:
                print(f"  [{i}] {v1} op {v2} = {actual_val} (expected: {expected_val})")

    if errors_found > 3:
        print(f"  ... and {errors_found - 3} more errors")

    test_result(f"{name} (errors: {errors_found})", all_passed)
    return all_passed

# Test integer add
try:
    test_int_array_operation("add_arrays_int_neon",
                            rp_dsp.add_arrays_int_neon,
                            lambda a, b: a + b)
except AttributeError as e:
    print(f"  [SKIP] add_arrays_int_neon not available: {e}")

# Test integer subtract
try:
    test_int_array_operation("subtract_arrays_int_neon",
                            rp_dsp.subtract_arrays_int_neon,
                            lambda a, b: a - b)
except AttributeError as e:
    print(f"  [SKIP] subtract_arrays_int_neon not available: {e}")

# Test integer multiply
try:
    test_int_array_operation("multiply_arrays_int_neon",
                            rp_dsp.multiply_arrays_int_neon,
                            lambda a, b: a * b)
except AttributeError as e:
    print(f"  [SKIP] multiply_arrays_int_neon not available: {e}")

# ============================================================================
# SECTION 7: Array Arithmetic Tests - Int16
# ============================================================================

test_section_header("Array Arithmetic Tests - Int16 (int16_t*)")

def test_int16_array_operation(name, func, scalar_op, size=SIZE):
    """Test int16 array operations with wraparound handling"""
    print(f"\n--- {name} ---")

    src1 = rp_dsp.arrInt16(size)
    src2 = rp_dsp.arrInt16(size)
    dst = rp_dsp.arrInt16(size)

    test_data = []
    for i in range(size):
        v1 = random.randint(-10000, 10000)
        v2 = random.randint(-10000, 10000)
        src1[i] = v1
        src2[i] = v2
        test_data.append((v1, v2))

    func(dst.cast(), src1.cast(), src2.cast(), size)

    def to_int16(val):
        """Convert to int16 with wraparound"""
        val = val & 0xFFFF
        if val > 32767:
            val -= 65536
        return val

    all_passed = True
    errors_found = 0
    for i in range(size):
        v1, v2 = test_data[i]
        expected_val = to_int16(scalar_op(v1, v2))
        actual_val = dst[i]
        if actual_val != expected_val:
            all_passed = False
            errors_found += 1
            if errors_found <= 3:
                print(f"  [{i}] {v1} op {v2} = {actual_val} (expected: {expected_val})")

    if errors_found > 3:
        print(f"  ... and {errors_found - 3} more errors")

    test_result(f"{name} (errors: {errors_found})", all_passed)
    return all_passed

# Test int16 add
try:
    test_int16_array_operation("add_arrays_int16_neon",
                              rp_dsp.add_arrays_int16_neon,
                              lambda a, b: a + b)
except AttributeError as e:
    print(f"  [SKIP] add_arrays_int16_neon not available: {e}")

# Test int16 subtract
try:
    test_int16_array_operation("subtract_arrays_int16_neon",
                              rp_dsp.subtract_arrays_int16_neon,
                              lambda a, b: a - b)
except AttributeError as e:
    print(f"  [SKIP] subtract_arrays_int16_neon not available: {e}")

# Test int16 multiply
try:
    test_int16_array_operation("multiply_arrays_int16_neon",
                              rp_dsp.multiply_arrays_int16_neon,
                              lambda a, b: a * b)
except AttributeError as e:
    print(f"  [SKIP] multiply_arrays_int16_neon not available: {e}")

# ============================================================================
# SECTION 8: Array Arithmetic Tests - Double
# ============================================================================

test_section_header("Array Arithmetic Tests - Double (double*)")

def test_double_array_operation(name, func, scalar_op, size=SIZE):
    """Test double array operations"""
    print(f"\n--- {name} ---")

    src1 = rp_dsp.arrDouble(size)
    src2 = rp_dsp.arrDouble(size)
    dst = rp_dsp.arrDouble(size)

    test_data = []
    for i in range(size):
        v1 = random.uniform(-100.0, 100.0)
        v2 = random.uniform(-100.0, 100.0)
        src1[i] = v1
        src2[i] = v2
        test_data.append((v1, v2))

    func(dst.cast(), src1.cast(), src2.cast(), size)

    all_passed = True
    max_error = 0.0
    errors_found = 0

    for i in range(size):
        v1, v2 = test_data[i]
        expected_val = scalar_op(v1, v2)
        actual_val = dst[i]
        error = abs(actual_val - expected_val)
        if error > max_error:
            max_error = error
        if not compare_double(actual_val, expected_val, 1e-10):
            all_passed = False
            errors_found += 1
            if errors_found <= 3:
                print(f"  [{i}] {v1:.8f} op {v2:.8f} = {actual_val:.12f} (expected: {expected_val:.12f}) error: {error:.2e}")

    if errors_found > 3:
        print(f"  ... and {errors_found - 3} more errors")

    test_result(f"{name} (errors: {errors_found}, max error: {max_error:.2e})", all_passed)
    return all_passed

# Test double add
try:
    test_double_array_operation("add_arrays_double_neon",
                               rp_dsp.add_arrays_double_neon,
                               lambda a, b: a + b)
except AttributeError as e:
    print(f"  [SKIP] add_arrays_double_neon not available: {e}")

# Test double subtract
try:
    test_double_array_operation("subtract_arrays_double_neon",
                               rp_dsp.subtract_arrays_double_neon,
                               lambda a, b: a - b)
except AttributeError as e:
    print(f"  [SKIP] subtract_arrays_double_neon not available: {e}")

# Test double multiply
try:
    test_double_array_operation("multiply_arrays_double_neon",
                               rp_dsp.multiply_arrays_double_neon,
                               lambda a, b: a * b)
except AttributeError as e:
    print(f"  [SKIP] multiply_arrays_double_neon not available: {e}")

# Test double divide
try:
    print(f"\n--- divide_arrays_double_neon ---")
    src1 = rp_dsp.arrDouble(SIZE)
    src2 = rp_dsp.arrDouble(SIZE)
    dst = rp_dsp.arrDouble(SIZE)

    test_data = []
    for i in range(SIZE):
        v1 = random.uniform(-100.0, 100.0)
        v2 = random.uniform(-100.0, 100.0)
        if abs(v2) < 0.001:
            v2 = 1.0
        src1[i] = v1
        src2[i] = v2
        test_data.append((v1, v2))

    rp_dsp.divide_arrays_double_neon(dst.cast(), src1.cast(), src2.cast(), SIZE)

    all_passed = True
    max_error = 0.0
    for i in range(SIZE):
        v1, v2 = test_data[i]
        expected_val = v1 / v2
        actual_val = dst[i]
        error = abs(actual_val - expected_val)
        if error > max_error:
            max_error = error
        if not compare_double(actual_val, expected_val, 1e-6):
            all_passed = False

    test_result(f"divide_arrays_double_neon (max error: {max_error:.2e})", all_passed)
except AttributeError as e:
    print(f"  [SKIP] divide_arrays_double_neon not available: {e}")

# ============================================================================
# SECTION 9: Scalar-Array Operations - Double
# ============================================================================

test_section_header("Scalar-Array Operations - Double")

def test_scalar_double_operation(name, func, scalar_op, scalar_val, size=SIZE):
    """Test scalar-double array operations"""
    print(f"\n--- {name} (scalar={scalar_val}) ---")

    src = rp_dsp.arrDouble(size)
    dst = rp_dsp.arrDouble(size)

    test_data = []
    for i in range(size):
        v = random.uniform(-100.0, 100.0)
        src[i] = v
        test_data.append(v)

    func(dst.cast(), src.cast(), scalar_val, size)

    all_passed = True
    max_error = 0.0
    errors_found = 0

    for i in range(size):
        expected_val = scalar_op(test_data[i], scalar_val)
        actual_val = dst[i]
        error = abs(actual_val - expected_val)
        if error > max_error:
            max_error = error
        if not compare_double(actual_val, expected_val, 1e-10):
            all_passed = False
            errors_found += 1
            if errors_found <= 3:
                print(f"  [{i}] {test_data[i]:.8f} op {scalar_val} = {actual_val:.12f} (expected: {expected_val:.12f})")

    if errors_found > 3:
        print(f"  ... and {errors_found - 3} more errors")

    test_result(f"{name} (errors: {errors_found}, max error: {max_error:.2e})", all_passed)
    return all_passed

# Test double scalar add
try:
    test_scalar_double_operation("add_scalar_to_array_double_neon",
                                rp_dsp.add_scalar_to_array_double_neon,
                                lambda a, s: a + s, 5.0)
except AttributeError as e:
    print(f"  [SKIP] add_scalar_to_array_double_neon not available: {e}")

# Test double scalar subtract
try:
    test_scalar_double_operation("subtract_scalar_from_array_double_neon",
                                rp_dsp.subtract_scalar_from_array_double_neon,
                                lambda a, s: a - s, 3.0)
except AttributeError as e:
    print(f"  [SKIP] subtract_scalar_from_array_double_neon not available: {e}")

# Test double scalar multiply
try:
    test_scalar_double_operation("multiply_array_by_scalar_double_neon",
                                rp_dsp.multiply_array_by_scalar_double_neon,
                                lambda a, s: a * s, 2.5)
except AttributeError as e:
    print(f"  [SKIP] multiply_array_by_scalar_double_neon not available: {e}")

# Test double scalar divide
try:
    test_scalar_double_operation("divide_array_by_scalar_double_neon",
                                rp_dsp.divide_array_by_scalar_double_neon,
                                lambda a, s: a / s if s != 0 else float('inf'), 4.0)
except AttributeError as e:
    print(f"  [SKIP] divide_array_by_scalar_double_neon not available: {e}")

# ============================================================================
# SECTION 10: Divide with Zero Handling
# ============================================================================

test_section_header("Divide Arrays with Zero Handling (divide_arrays_neon_Ex)")

try:
    print("\n--- divide_arrays_neon_Ex Tests ---")

    SIZE_DIV = 32
    limit = 1000.0

    src1 = rp_dsp.arrFloat(SIZE_DIV)
    src2 = rp_dsp.arrFloat(SIZE_DIV)
    dst = rp_dsp.arrFloat(SIZE_DIV)

    # Test cases
    test_pairs = [
        (10.0, 2.0, 5.0),
        (10.0, 0.0, limit),
        (-10.0, 0.0, -limit),
        (0.0, 0.0, -limit),
        (100.0, 4.0, 25.0),
        (-100.0, 0.0, -limit),
        (1.0, 0.0, limit),
        (-1.0, 0.0, -limit),
        (0.0, 1.0, 0.0),
        (123.456, 2.0, 61.728),
        (-500.0, 0.0, -limit),
        (1000000.0, 0.0, limit),
    ]

    for i, (v1, v2, exp) in enumerate(test_pairs):
        src1[i] = v1
        src2[i] = v2

    rp_dsp.divide_arrays_neon_Ex(dst.cast(), src1.cast(), src2.cast(), len(test_pairs), limit)

    all_passed = True
    for i, (v1, v2, exp) in enumerate(test_pairs):
        actual = dst[i]
        passed = compare_float(actual, exp, 1e-3)
        if not passed:
            all_passed = False
        print(f"  [{i:2d}] {v1:10.4f} / {v2:10.4f} = {actual:12.6f} (expected: {exp:12.6f}) {'✓' if passed else '✗'}")

    test_result("divide_arrays_neon_Ex", all_passed)

except AttributeError as e:
    print(f"  [SKIP] divide_arrays_neon_Ex not available: {e}")

# ============================================================================
# SECTION 11: Cleanup
# ============================================================================

test_section_header("Cleanup")

print("\n--- Cleanup ---")

# No explicit deleteData function exists - Python garbage collector handles cleanup
# The CDSP destructor will be called automatically when obj goes out of scope
print("  Python garbage collector will handle CDSP cleanup")

# If you need explicit cleanup, just delete the reference
print("  Deleting data reference...")
data = None
test_result("Data reference cleared", True)

print("  Deleting DSP object reference...")
obj = None
test_result("DSP object reference cleared", True)

print("\n" + "=" * 80)
print("All tests completed!")
print("=" * 80)