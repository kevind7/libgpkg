#ifndef GPKG_GEOS_CONTEXT_H
#define GPKG_GEOS_CONTEXT_H

#include <geos_c.h>
#include "error.h"

void geom_geos_clear_error();

void geom_geos_print_error();

void geom_geos_get_error(error_t *error);

GEOSContextHandle_t geom_geos_init();

#endif