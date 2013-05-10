/*
 * Copyright 2013 Luciad (http://www.luciad.com)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <float.h>
#include <string.h>
#include "gpb.h"
#include "sqlite.h"



static size_t gpb_header_size(gpb_header_t *gpb) {
    return (size_t)
            4 // Magic number
            + 4 // SRID
            + 8 * ((gpb->envelope.has_env_x ? 2 : 0) + (gpb->envelope.has_env_y ? 2 : 0) + (gpb->envelope.has_env_z ? 2 : 0) + (gpb->envelope.has_env_m ? 2 : 0)); // Envelope
}

int gpb_read_header(binstream_t *stream, gpb_header_t *gpb) {
    uint8_t head[3];
    if (binstream_nread_u8(stream, head, 3)) {
        return SQLITE_IOERR;
    }

    if (memcmp(head, "GPB", 3) != 0) {
        return SQLITE_IOERR;
    }

    uint8_t flags;
    if (binstream_read_u8(stream, &flags)) {
        return SQLITE_IOERR;
    }

    gpb->version = (flags & 0xF0) >> 4;
    uint8_t envelope = (flags & 0xE) >> 1;
    uint8_t endian = flags & 0x1;

    if (gpb->version != 1) {
        return SQLITE_IOERR;
    }

    if (envelope > 4) {
        return SQLITE_IOERR;
    }

    binstream_set_endianness(stream, endian == 0 ? BIG : LITTLE);

    if (binstream_read_u32(stream, &gpb->srid)) {
        return SQLITE_IOERR;
    }

    if (envelope > 0) {
        gpb->envelope.has_env_x = 1;
        if (binstream_read_double(stream, &gpb->envelope.min_x)) {
            return SQLITE_IOERR;
        }
        if (binstream_read_double(stream, &gpb->envelope.max_x)) {
            return SQLITE_IOERR;
        }
        gpb->envelope.has_env_y = 1;
        if (binstream_read_double(stream, &gpb->envelope.min_y)) {
            return SQLITE_IOERR;
        }
        if (binstream_read_double(stream, &gpb->envelope.max_y)) {
            return SQLITE_IOERR;
        }
    } else {
        gpb->envelope.has_env_x = 0;
        gpb->envelope.min_x = 0.0;
        gpb->envelope.max_x = 0.0;
        gpb->envelope.has_env_y = 0;
        gpb->envelope.min_y = 0.0;
        gpb->envelope.max_y = 0.0;
    }

    if (envelope == 2 || envelope == 4) {
        gpb->envelope.has_env_z = 1;
        if (binstream_read_double(stream, &gpb->envelope.min_z)) {
            return SQLITE_IOERR;
        }
        if (binstream_read_double(stream, &gpb->envelope.max_z)) {
            return SQLITE_IOERR;
        }
    } else {
        gpb->envelope.has_env_z = 0;
        gpb->envelope.min_z = 0.0;
        gpb->envelope.max_z = 0.0;
    }

    if (envelope == 3 || envelope == 4) {
        gpb->envelope.has_env_m = 1;
        if (binstream_read_double(stream, &gpb->envelope.min_m)) {
            return SQLITE_IOERR;
        }
        if (binstream_read_double(stream, &gpb->envelope.max_m)) {
            return SQLITE_IOERR;
        }
    } else {
        gpb->envelope.has_env_m = 0;
        gpb->envelope.min_m = 0.0;
        gpb->envelope.max_m = 0.0;
    }

    return SQLITE_OK;
}

int gpb_write_header(binstream_t *stream, gpb_header_t *gpb) {
    if (binstream_write_nu8(stream, (uint8_t*)"GPB", 3)) {
        return SQLITE_IOERR;
    }

    uint8_t version = gpb->version;
    uint8_t envelope = 0;
    if (gpb->envelope.has_env_x && gpb->envelope.has_env_y) {
        envelope = 1;
        if (gpb->envelope.has_env_z && gpb->envelope.has_env_m) {
            envelope = 4;
        } else if (gpb->envelope.has_env_z) {
            envelope = 2;
        } else if (gpb->envelope.has_env_m) {
            envelope = 3;
        }
    }
    uint8_t endian = binstream_get_endianness(stream) == LITTLE ? 1 : 0;

    uint8_t flags = (version << 4) | (envelope << 1) | endian;
    if (binstream_write_u8(stream, flags)) {
        return SQLITE_IOERR;
    }

    if (binstream_write_u32(stream, gpb->srid)) {
        return SQLITE_IOERR;
    }

    if (gpb->envelope.has_env_x) {
        if (binstream_write_double(stream, gpb->envelope.min_x)) {
            return SQLITE_IOERR;
        }
        if (binstream_write_double(stream, gpb->envelope.max_x)) {
            return SQLITE_IOERR;
        }
    }

    if (gpb->envelope.has_env_y) {
        if (binstream_write_double(stream, gpb->envelope.min_y)) {
            return SQLITE_IOERR;
        }
        if (binstream_write_double(stream, gpb->envelope.max_y)) {
            return SQLITE_IOERR;
        }
    }

    if (gpb->envelope.has_env_z) {
        if (binstream_write_double(stream, gpb->envelope.min_z)) {
            return SQLITE_IOERR;
        }
        if (binstream_write_double(stream, gpb->envelope.max_z)) {
            return SQLITE_IOERR;
        }
    }

    if (gpb->envelope.has_env_m) {
        if (binstream_write_double(stream, gpb->envelope.min_m)) {
            return SQLITE_IOERR;
        }
        if (binstream_write_double(stream, gpb->envelope.max_m)) {
            return SQLITE_IOERR;
        }
    }

    return SQLITE_OK;
}

static void gpb_begin(geom_consumer_t *consumer, geom_header_t *header) {
    gpb_writer_t *writer = (gpb_writer_t *) consumer;
    wkb_writer_t *wkb = &writer->wkb_writer;

    if (wkb->offset < 0) {
        gpb_header_t *gpb_header = &writer->header;
        if (header->geom_type != GEOM_POINT) {
            switch(header->coord_type) {
                case GEOM_XYZ:
                    gpb_header->envelope.has_env_x = 1;
                    gpb_header->envelope.has_env_y = 1;
                    gpb_header->envelope.has_env_z = 1;
                    break;
                case GEOM_XYM:
                    gpb_header->envelope.has_env_x = 1;
                    gpb_header->envelope.has_env_y = 1;
                    gpb_header->envelope.has_env_m = 1;
                    break;
                case GEOM_XYZM:
                    gpb_header->envelope.has_env_x = 1;
                    gpb_header->envelope.has_env_y = 1;
                    gpb_header->envelope.has_env_z = 1;
                    gpb_header->envelope.has_env_m = 1;
                    break;
                default:
                    gpb_header->envelope.has_env_x = 1;
                    gpb_header->envelope.has_env_y = 1;
                    break;
            }
        }
        binstream_relseek(&wkb->stream, gpb_header_size(gpb_header));
    }

    geom_consumer_t *wkb_consumer = wkb_writer_geom_consumer(wkb);
    wkb_consumer->begin(wkb_consumer, header);
}

static void gpb_coordinates(geom_consumer_t *consumer, geom_header_t *header, size_t point_count, double *coords) {
    gpb_writer_t *writer = (gpb_writer_t *) consumer;
    wkb_writer_t *wkb = &writer->wkb_writer;
    geom_consumer_t *wkb_consumer = wkb_writer_geom_consumer(wkb);
    wkb_consumer->coordinates(wkb_consumer, header, point_count, coords);

    gpb_header_t *gpb = &writer->header;
    int offset = 0;
    switch(header->coord_type) {
        #define MIN_MAX(coord) double coord = coords[offset++]; \
        if (coord < gpb->envelope.min_##coord) gpb->envelope.min_##coord = coord; \
        if (coord > gpb->envelope.max_##coord) gpb->envelope.max_##coord = coord;
        case GEOM_XYZ:
            for(int i = 0; i < point_count; i++) {
                MIN_MAX(x)
                MIN_MAX(y)
                MIN_MAX(z)
            }
            break;
        case GEOM_XYM:
            for(int i = 0; i < point_count; i++) {
                MIN_MAX(x)
                MIN_MAX(y)
                MIN_MAX(m)
            }
            break;
        case GEOM_XYZM:
            for(int i = 0; i < point_count; i++) {
                MIN_MAX(x)
                MIN_MAX(y)
                MIN_MAX(z)
                MIN_MAX(m)
            }
            break;
        default:
            for(int i = 0; i < point_count; i++) {
                MIN_MAX(x)
                MIN_MAX(y)
            }
            break;
        #undef MIN_MAX
    }
}

static void gpb_end(geom_consumer_t *consumer, geom_header_t *header) {
    gpb_writer_t *writer = (gpb_writer_t *) consumer;
    wkb_writer_t *wkb = &writer->wkb_writer;
    binstream_t *stream = &wkb->stream;

    geom_consumer_t *wkb_consumer = wkb_writer_geom_consumer(wkb);
    wkb_consumer->end(wkb_consumer, header);

    if (wkb->offset < 0) {
        binstream_seek(stream, 0);
        gpb_write_header(stream, &writer->header);
        binstream_seek(stream, 0);
    }
}

int gpb_writer_init( gpb_writer_t *writer, uint32_t srid ) {
    geom_consumer_init(&writer->geom_consumer, gpb_begin, gpb_end, gpb_coordinates);
    geom_envelope_init(&writer->header.envelope);
    writer->header.version = 1;
    writer->header.srid = srid;
    return wkb_writer_init(&writer->wkb_writer);
}

geom_consumer_t * gpb_writer_geom_consumer(gpb_writer_t *writer) {
    return &writer->geom_consumer;
}

void gpb_writer_destroy( gpb_writer_t *writer ) {
    wkb_writer_destroy(&writer->wkb_writer);
}

uint8_t* gpb_writer_getgpb( gpb_writer_t *writer ) {
    return binstream_data(&writer->wkb_writer.stream);
}

size_t gpb_writer_length( gpb_writer_t *writer ) {
    return binstream_available(&writer->wkb_writer.stream);
}
