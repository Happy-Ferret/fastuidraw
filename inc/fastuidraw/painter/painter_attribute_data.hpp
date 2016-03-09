/*!
 * \file painter_attribute_data.hpp
 * \brief file painter_attribute_data.hpp
 *
 * Copyright 2016 by Intel.
 *
 * Contact: kevin.rogovin@intel.com
 *
 * This Source Code Form is subject to the
 * terms of the Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with
 * this file, You can obtain one at
 * http://mozilla.org/MPL/2.0/.
 *
 * \author Kevin Rogovin <kevin.rogovin@intel.com>
 *
 */


#pragma once

#include <fastuidraw/util/util.hpp>
#include <fastuidraw/stroked_path.hpp>
#include <fastuidraw/filled_path.hpp>
#include <fastuidraw/painter/painter_attribute.hpp>
#include <fastuidraw/painter/painter_enums.hpp>
#include <fastuidraw/text/glyph.hpp>

namespace fastuidraw
{
/*!\addtogroup Painter
  @{
 */

  /*!
    PainterAttributeData represents the attribute and index
    data ready to be consumed by a Painter. Data is organized
    into individual chuncks that can be drawn seperately.

    PainterAttributeData provides methods to set its attribute and
    index data from stroking paths or filling paths.

    Data for stroking is packed as follows:
     - PainterAttribute::m_primary_attrib .xy -> StrokedPath::point::m_position
     - PainterAttribute::m_primary_attrib .zw -> StrokedPath::point::m_pre_offset
     - PainterAttribute::m_secondary_attrib .x -> StrokedPath::point::m_distance_from_edge_start
     - PainterAttribute::m_secondary_attrib .y -> StrokedPath::point::m_distance_from_outline_start
     - PainterAttribute::m_secondary_attrib .z -> StrokedPath::point::m_miter_distance
     - PainterAttribute::m_secondary_attrib .w -> StrokedPath::point::m_on_boundary
     - PainterAttribute::m_uint_attrib .x -> StrokedPath::point::m_depth
     - PainterAttribute::m_uint_attrib .y -> 0 (free)
     - PainterAttribute::m_uint_attrib .z -> 0 (free)
     - PainterAttribute::m_uint_attrib .w -> 0 (free)

    Data for filling is packed as follows:
     - PainterAttribute::m_primary_attrib .xy    -> coordinate of point.
     - PainterAttribute::m_primary_attrib .zw    -> 0.0 (free)
     - PainterAttribute::m_secondary_attrib .xyz -> 0.0 (free)
     - PainterAttribute::m_secondary_attrib .w   -> 0.0 (free)
     - PainterAttribute::m_uint_attrib .x -> 0 (free)
     - PainterAttribute::m_uint_attrib .y -> 0 (free)
     - PainterAttribute::m_uint_attrib .z -> 0 (free)
     - PainterAttribute::m_uint_attrib .w -> 0 (free)

    Data for glyphs is packed as follows:
     - PainterAttribute::m_primary_attrib .xy   -> xy-texel location in primary atlas
     - PainterAttribute::m_primary_attrib .zw   -> xy-texel location in secondary atlas
     - PainterAttribute::m_secondary_attrib .xy -> position in item coordinates
     - PainterAttribute::m_secondary_attrib .z  -> 0.0 (free)
     - PainterAttribute::m_secondary_attrib .w  -> 0.0 (free)
     - PainterAttribute::m_uint_attrib .x -> 0
     - PainterAttribute::m_uint_attrib .y -> glyph offset
     - PainterAttribute::m_uint_attrib .z -> layer in primary atlas
     - PainterAttribute::m_uint_attrib .w -> layer in secondary atlas
   */
  class PainterAttributeData:noncopyable
  {
  public:
    /*!
      Enumation values are indexes into attribute_data_chunks()
      and index_data_chunks() for different portions of
      data needed for stroking a path when the data of this
      PainterAttributeData has been set with
      set_data(const StrokedPath::const_handle &).
     */
    enum stroking_data_t
      {
        rounded_joins_closing_edge, /*!< index for rounded join data with closing edge */
        bevel_joins_closing_edge, /*!< index for bevel join data with closing edge */
        miter_joins_closing_edge, /*!< index for miter join data with closing edge */
        edge_closing_edge, /*!< index for edge data including closing edge */

        number_with_closing_edge, /*!< number of types with closing edge */

        rounded_joins_no_closing_edge = number_with_closing_edge, /*!< index for rounded join data without closing edge */
        bevel_joins_no_closing_edge, /*!< index for bevel join data without closing edge */
        miter_joins_no_closing_edge, /*!< index for miter join data without closing edge */
        edge_no_closing_edge, /*!< index for edge data not including closing edge */

        rounded_cap, /*!< index for rounded cap data */
        square_cap,  /*!< index for square cap data */

        stroking_data_count /*!< count of enums */
      };

    /*!
      Given an enumeration of stroking_data_t, returns
      the matching enumeration for drawing without the
      closing edge.
     */
    static
    enum stroking_data_t
    without_closing_edge(enum stroking_data_t v)
    {
      return (v < number_with_closing_edge) ?
        static_cast<enum stroking_data_t>(v + number_with_closing_edge) :
        v;
    }

    /*!
      Ctor.
     */
    PainterAttributeData(void);

    ~PainterAttributeData();

    /*!
      Set the attribute and index data for stroking a path.
      The enumerations of \ref stroking_data_t provide
      the indices into attribute_data_chunks() and
      index_data_chunks() for the data to draw the
      path stroked
      \param path path to stroke
     */
    void
    set_data(const StrokedPath::const_handle &path);

    /*!
      Set the attribute and index data for filling a path.
      The enumeration values of PainterEnums::fill_rule_t provide
      the indices into attribute_data_chunks() and
      index_data_chunks() to use for the fill rules.
      \param path path to fill
     */
    void
    set_data(const FilledPath::const_handle &path);

    /*!
      Set the data for drawing glyphs. The enumeration glyph_type
      provide the indices into attribute_data_chunks() and
      index_data_chunks() for the different glyph types.
      If a glyph is not uploaded to the GlyphCache and failed to
      be uploaded to its GlyphCache, then set_data() will create
      index and attribute data up to that glyph and returns the
      index into glyphs of the glyph that failed to be uploaded.
      If all glyphs can be in the cache, then returns the
      size of the array.
      \param glyph_positions position of the bottom left corner of each glyph
      \param glyphs glyphs to draw, array must be same size as glyph_positions
      \param scale_factors scale factors to apply to each glyph, must be either
                           empty (indicating no scaling factors) or the exact
                           same length as glyph_positions
      \param orientation orientation of drawing
     */
    unsigned int
    set_data(const_c_array<vec2> glyph_positions,
             const_c_array<Glyph> glyphs,
             const_c_array<float> scale_factors,
             enum PainterEnums::glyph_orientation orientation = PainterEnums::y_increases_downwards);

    /*!
      Set the data for drawing glyphs. The enumeration glyph_type
      provide the indices into attribute_data_chunks() and
      index_data_chunks() for the different glyph types.
      If a glyph is not uploaded to the GlyphCache and failed to
      be uploaded to its GlyphCache, then set_data() will create
      index and attribute data up to that glyph and returns the
      index into glyphs of the glyph that failed to be uploaded.
      If all glyphs can be in the cache, then returns the
      size of the array.
      \param glyph_positions position of the bottom left corner of each glyph
      \param glyphs glyphs to draw, array must be same size as glyph_positions
      \param render_pixel_size pixel size to which to scale the glyphs
      \param orientation orientation of drawing
     */
    unsigned int
    set_data(const_c_array<vec2> glyph_positions,
             const_c_array<Glyph> glyphs,
             float render_pixel_size,
             enum PainterEnums::glyph_orientation orientation = PainterEnums::y_increases_downwards);

    /*!
      Set the data for drawing glyphs. The enumeration glyph_type
      provide the indices into attribute_data_chunks() and
      index_data_chunks() for the different glyph types.
      If a glyph is not uploaded to the GlyphCache and failed to
      be uploaded to its GlyphCache, then set_data() will create
      index and attribute data up to that glyph and returns the
      index into glyphs of the glyph that failed to be uploaded.
      If all glyphs can be in the cache, then returns the
      size of the array.
      \param glyph_positions position of the bottom left corner of each glyph
      \param glyphs glyphs to draw, array must be same size as glyph_positions
      \param orientation orientation of drawing
     */
    unsigned int
    set_data(const_c_array<vec2> glyph_positions,
             const_c_array<Glyph> glyphs,
             enum PainterEnums::glyph_orientation orientation = PainterEnums::y_increases_downwards)
    {
      c_array<float> empty;
      return set_data(glyph_positions, glyphs, empty, orientation);
    }

    /*!
      Returns the attribute data chunks. For each
      attribute data chunk, there is a matching index
      data chunk. A chunk is an attribute and index
      data chunk pair. Specifically one uses
      index_data_chunks()[i] to draw the contents
      of attribute_data_chunks()[i].
     */
    const_c_array<const_c_array<PainterAttribute> >
    attribute_data_chunks(void) const;

    /*!
      Provided as an API conveniance to fetch
      the named chunk of attribute_data_chunks()
      or an empty chunk if the index is larger then
      attribute_data_chunks().size().
      \param i index of attribute_data_chunks() to fetch
     */
    const_c_array<PainterAttribute>
    attribute_data_chunk(unsigned int i) const;

    /*!
      Returns the index data chunks. For each
      index data chunk, there is a matching
      attribute data chunk. Specifically one uses
      index_data_chunks()[i] to draw the contents
      of attribute_data_chunks()[i].
    */
    const_c_array<const_c_array<PainterIndex> >
    index_data_chunks(void) const;

    /*!
      Provided as an API conveniance to fetch
      the named chunk of index_data_chunks()
      or an empty chunk if the index is larger then
      index_data_chunks().size().
      \param i index of index_data_chunks() to fetch
     */
    const_c_array<PainterIndex>
    index_data_chunk(unsigned int i) const;

    /*!
      Returns an array that holds those value i
      for which index_data_chunk(i) is non-empty.
     */
    const_c_array<unsigned int>
    non_empty_index_data_chunks(void) const;

    /*!
      Returns by how much to increment a z-value
      (see Painter::increment_z()) when using
      an attribute/index pair.
     */
    const_c_array<unsigned int>
    increment_z_values(void) const;

    /*!
      Provided as an API conveniance to fetch
      the named increment-z value of
      increment_z_values() or 0 if the index is
      larger then increment_z_values().size().
      \param i index of increment_z_values() to fetch
     */
    unsigned int
    increment_z_value(unsigned int i) const;

  private:
    void *m_d;
  };
/*! @} */
}