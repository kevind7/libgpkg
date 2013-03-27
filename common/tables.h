#ifndef GPKG_TABLES_H
#define GPKG_TABLES_H

#include <stdlib.h>
#include "sql.h"

#define N NULL_VALUE
#define D(v) DOUBLE_VALUE(v)
#define I(v) INT_VALUE(v)
#define T(v) TEXT_VALUE(v)
#define F(v) FUNC_VALUE(v)

column_info_t geopackage_contents_columns[] = {
        {"table_name", "text", N, SQL_PRIMARY_KEY | SQL_NOT_NULL, NULL},
        {"data_type", "text", N, SQL_NOT_NULL, NULL},
        {"identifier", "text", N, SQL_NOT_NULL, NULL},
        {"description", "text", N, SQL_NOT_NULL, NULL},
        {"last_change", "text", F("strftime('%Y-%m-%dT%H:%M:%fZ', 'now')"), SQL_NOT_NULL, NULL},
        {"min_x", "double", D(-180.0), SQL_NOT_NULL, NULL},
        {"min_y", "double", D(-90.0), SQL_NOT_NULL, NULL},
        {"max_x", "double", D(180.0), SQL_NOT_NULL, NULL},
        {"max_y", "double", D(90.0), SQL_NOT_NULL, NULL},
        {"srid", "integer", I(0), SQL_NOT_NULL, "REFERENCES spatial_ref_sys(srid)"},
};
table_info_t geopackage_contents = {
        "geopackage_contents",
        geopackage_contents_columns, 10,
        NULL, 0
};

column_info_t spatial_ref_sys_columns[] = {
        {"srid", "integer", N, SQL_PRIMARY_KEY, NULL},
        {"auth_name", "text", N, SQL_NOT_NULL, NULL},
        {"auth_srid", "integer", N, SQL_NOT_NULL, NULL},
        {"srtext", "text", N, SQL_NOT_NULL, NULL}
};
value_t spatial_ref_sys_data[] = {
        I(-1), T("NONE"), I(-1), T("undefined"),
        I(0), T("NONE"), I(0), T("undefined")
};
table_info_t spatial_ref_sys = {
        "spatial_ref_sys",
        spatial_ref_sys_columns, 4,
        spatial_ref_sys_data, 2
};

column_info_t geometry_columns_columns[] = {
        {"f_table_name", "text", N, SQL_PRIMARY_KEY, "REFERENCES geopackage_contents(table_name)"},
        {"f_geometry_column", "text", N, SQL_PRIMARY_KEY, NULL},
        {"geometry_type", "integer", N, SQL_NOT_NULL, NULL},
        {"srid", "integer", N, SQL_NOT_NULL, "REFERENCES spatial_ref_sys(srid)"}
};
table_info_t geometry_columns = {
        "geometry_columns",
        geometry_columns_columns, 4,
        NULL, 0
};

column_info_t raster_columns_columns[] = {
        {"r_table_name", "text", N, SQL_PRIMARY_KEY, "REFERENCES geopackage_contents(table_name)"},
        {"r_raster_column", "text", N, SQL_PRIMARY_KEY, NULL},
        {"compr_qual_factor", "integer", I(-1), SQL_NOT_NULL, NULL},
        {"georectification", "integer", I(-1), SQL_NOT_NULL, NULL},
        {"srid", "integer", N, SQL_NOT_NULL, "REFERENCES spatial_ref_sys(srid)"}
};
table_info_t raster_columns = {
        "raster_columns",
        raster_columns_columns, 5,
        NULL, 0
};

column_info_t tile_table_metadata_columns[] = {
        {"t_table_name", "text", N, SQL_PRIMARY_KEY, "REFERENCES geopackage_contents(table_name)"},
        {"is_times_two_zoom", "integer", I(1), SQL_NOT_NULL, NULL}
};
table_info_t tile_table_metadata = {
        "tile_table_metadata",
        tile_table_metadata_columns, 2,
        NULL, 0
};

column_info_t tile_matrix_metadata_columns[] = {
        {"t_table_name", "text", N, SQL_PRIMARY_KEY, "REFERENCES tile_table_metadata(t_table_name)"},
        {"zoom_level", "integer", I(0), SQL_PRIMARY_KEY, NULL},
        {"matrix_width", "integer", I(1), SQL_NOT_NULL, NULL},
        {"matrix_height", "integer", I(1), SQL_NOT_NULL, NULL},
        {"tile_width", "integer", I(256), SQL_NOT_NULL, NULL},
        {"tile_height", "integer", I(256), SQL_NOT_NULL, NULL},
        {"pixel_x_size", "double", D(1.0), SQL_NOT_NULL, NULL},
        {"pixel_y_size", "double", D(1.0), SQL_NOT_NULL, NULL}
};
table_info_t tile_matrix_metadata = {
        "tile_matrix_metadata",
        tile_matrix_metadata_columns, 8,
        NULL, 0
};

column_info_t xml_metadata_columns[] = {
        {"id", "integer", N, SQL_PRIMARY_KEY, NULL},
        {"md_scope", "text", T("undefined"), SQL_NOT_NULL, NULL},
        {"md_standard_uri", "text", T("http://schemas.opengis.net/iso/19139"), SQL_NOT_NULL, NULL},
        {"metadata", "text", T(""), SQL_NOT_NULL, NULL}
};
table_info_t xml_metadata = {
        "xml_metadata",
        xml_metadata_columns, 4,
        NULL, 0
};

column_info_t metadata_reference_columns[] = {
        {"reference_scope", "text", T("table"), SQL_NOT_NULL, NULL},
        {"table_name", "text", T("undefined"), SQL_NOT_NULL, NULL},
        {"column_name", "text", T("undefined"), SQL_NOT_NULL, NULL},
        {"row_id_value", "integer", I(0), SQL_NOT_NULL, NULL},
        {"timestamp", "text", F("strftime('%Y-%m-%dT%H:%M:%fZ', 'now')"), SQL_NOT_NULL, NULL},
        {"md_file_id", "integer", I(0), SQL_NOT_NULL, "REFERENCES xml_metadata(id)"},
        {"md_parent_id", "integer", I(0), SQL_NOT_NULL, "REFERENCES xml_metadata(id)"}
};
table_info_t metadata_reference = {
        "metadata_reference",
        metadata_reference_columns, 7,
        NULL, 0
};

table_info_t *tables[] = {
        &geopackage_contents,
        &spatial_ref_sys,
        &geometry_columns,
        &raster_columns,
        &tile_table_metadata,
        &tile_matrix_metadata,
        &xml_metadata,
        &metadata_reference,
        NULL
};

#undef N
#undef D
#undef T
#undef I

#endif