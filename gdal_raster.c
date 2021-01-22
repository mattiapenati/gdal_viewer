/*
 * Copyright 2021 Mattia Penati <mattia.penati@protonmail.com>
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

#include "raster.h"
#include "gdal.h"
#include <string.h>

static void* load_png(GDALDatasetH dataset);
static void* load_jpeg2000(GDALDatasetH dataset);
static void* load_gtiff(GDALDatasetH dataset);

void* make_gdal_raster(const char* filename)
{
    GDALAllRegister();

    const unsigned int open_flags = GDAL_OF_RASTER | GDAL_OF_READONLY;
    const GDALDatasetH dataset = GDALOpenEx(filename, open_flags, NULL, NULL, NULL);
    if (dataset == NULL)
        return NULL;

    const GDALDriverH driver = GDALGetDatasetDriver(dataset);
    const char* driver_name = GDALGetDriverShortName(driver);

    void* raster = NULL;
    if (strcmp(driver_name, "PNG") == 0)
        raster = load_png(dataset);
    else if (strcmp(driver_name, "JP2OpenJPEG") == 0)
        raster = load_jpeg2000(dataset);
    else if (strcmp(driver_name, "GTiff") == 0)
        raster = load_gtiff(dataset);

    GDALClose(dataset);
    return raster;
}

static void* load_rgba_image(GDALDatasetH dataset)
{
    unsigned char* pixels = NULL;
    void* raster = NULL;

    static const sg_pixel_format DataTypeToPixelFormat[GDT_TypeCount] = {
        [GDT_Byte] = SG_PIXELFORMAT_RGBA8,
        [GDT_UInt16] = SG_PIXELFORMAT_RGBA16,
    };

    static const size_t ColorInterToOffset[GCI_Max] = {
        [GCI_RedBand] = 0,
        [GCI_GreenBand] = 1,
        [GCI_BlueBand] = 2,
        [GCI_AlphaBand] = 3,
    };

    const int width = GDALGetRasterXSize(dataset);
    const int height = GDALGetRasterYSize(dataset);
    const int band_count = GDALGetRasterCount(dataset);

    const GDALDataType data_type = GDALGetRasterDataType(GDALGetRasterBand(dataset, 1));
    const sg_pixel_format pixel_format = DataTypeToPixelFormat[data_type];
    if (pixel_format == 0)
        goto error;

    const size_t pixel_size = GDALGetDataTypeSizeBytes(data_type);
    const size_t image_size = 4 * width * height * pixel_size;
    pixels = malloc(image_size);
    if (pixels == NULL)
        goto error;
    if (band_count == 3)
        memset(pixels, ~(0x0), image_size);

    for (int band_index = 1; band_index <= band_count; ++band_index)
    {
        const GDALRasterBandH band = GDALGetRasterBand(dataset, band_index);
        const GDALDataType band_data_type = GDALGetRasterDataType(band);
        if (band_data_type != data_type)
            goto error;

        const GDALColorInterp color_interp = GDALGetRasterColorInterpretation(band);
        CPLErr error = CE_None;
        switch (color_interp)
        {
        case GCI_RedBand:
        case GCI_GreenBand:
        case GCI_BlueBand:
        case GCI_AlphaBand:
            error = GDALRasterIO(band, GF_Read, 0, 0, width, height,
                pixels + ColorInterToOffset[color_interp] * pixel_size, width, height,
                data_type, 4 * pixel_size, 4 * pixel_size * width);
            break;
        default:
            break;
        }
    }

    raster = make_rgba_raster(pixels, width, height, pixel_format);

error:
    free(pixels);
    return raster;
}

static void* load_png(GDALDatasetH dataset)
{
    const int band_count = GDALGetRasterCount(dataset);
    if (band_count == 3 || band_count == 4)
    {
        return load_rgba_image(dataset);
    }
    else
    {
        return NULL;
    }
}

static void* load_jpeg2000(GDALDatasetH dataset)
{
    return load_rgba_image(dataset);
}

static void* load_gtiff(GDALDatasetH dataset)
{
    const int band_count = GDALGetRasterCount(dataset);
    if (band_count == 3 || band_count == 4)
    {
        return load_rgba_image(dataset);
    }
    else
    {
        return NULL;
    }
}
