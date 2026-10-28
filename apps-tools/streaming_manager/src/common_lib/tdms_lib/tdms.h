#ifndef TDMS_LIB_TDMS_H
#define TDMS_LIB_TDMS_H

/**
 * The TDMS library: one include for all of it. See README.md for the guide.
 *
 *   Value, TimeStamp, Properties   property values (value.h)
 *   SegmentBuilder                 one segment: properties and samples, referenced or copied (segment_builder.h)
 *   FileWriter                     a file written segment by segment, samples straight from memory (file_writer.h)
 *   FileReader                     a file read by channel, samples straight into memory (file_reader.h)
 *   Waveform, setWaveform          the NI time properties of a channel (waveform.h)
 *
 * File, Reader and Writer (file.h) are the former API, kept until reader_lib and writer_lib
 * use this one.
 */
#include "file_reader.h"
#include "file_writer.h"
#include "segment_builder.h"
#include "value.h"
#include "waveform.h"

#endif
