/*!
 * \file tessellated_path.hpp
 * \brief file tessellated_path.hpp
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


#include <fastuidraw/util/fastuidraw_memory.hpp>
#include <fastuidraw/util/vecN.hpp>
#include <fastuidraw/util/c_array.hpp>
#include <fastuidraw/util/reference_counted.hpp>

namespace fastuidraw  {

///@cond
class Path;
class StrokedPath;
class FilledPath;
///@endcond

/*!\addtogroup Paths
  @{
 */

/*!
  \brief
  A TessellatedPath represents the tessellation of a Path.

  A single contour of a TessellatedPath is constructed from
  a single \ref PathContour of the source \ref Path. Each
  edge of a contour of a TessellatedPath is contructed from
  a single \ref PathContour::interpolator_base of the source \ref
  PathContour. The ordering of the contours of a
  TessellatedPath is the same ordering as the source
  \ref PathContour objects of the source \ref Path. Also,
  the ordering of edges within a contour is the same ordering
  as the \ref PathContour::interpolator_base objects of
  the source \ref PathContour. In particular, for each contour
  of a TessellatedPath, the closing edge is the last edge.
 */
class TessellatedPath:
    public reference_counted<TessellatedPath>::non_concurrent
{
public:
  /*!
    \brief
    A TessellationParams stores how finely to tessellate
    the curves of a path.
   */
  class TessellationParams
  {
  public:
    /*!
      Ctor, initializes values.
     */
    TessellationParams(void):
      m_curvature_tessellation(true),
      m_threshhold(float(M_PI)/30.0f),
      m_max_segments(32)
    {}

    /*!
      Non-equal comparison operator.
      \param rhs value to which to compare against
     */
    bool
    operator!=(const TessellationParams &rhs) const
    {
      return m_curvature_tessellation != rhs.m_curvature_tessellation
        || m_threshhold != rhs.m_threshhold
        || m_max_segments != rhs.m_max_segments;
    }

    /*!
      Provided as a conveniance. Equivalent to
      \code
      m_curvature_tessellation = true;
      m_threshhold = p;
      \endcode
      \param p value to which to assign to \ref m_threshhold
     */
    TessellationParams&
    curvature_tessellate(float p)
    {
      m_curvature_tessellation = true;
      m_threshhold = p;
      return *this;
    }

    /*!
      Provided as a conveniance. Equivalent to
      \code
      m_curvature_tessellation = true;
      m_threshhold = 2.0f * static_cast<float>(M_PI) / static_cast<float>(N);
      \endcode
      \param N number of points for goal to tessellate a circle to.
     */
    TessellationParams&
    curvature_tessellate_num_points_in_circle(unsigned int N)
    {
      m_curvature_tessellation = true;
      m_threshhold = 2.0f * static_cast<float>(M_PI) / static_cast<float>(N);
      return *this;
    }

    /*!
      Provided as a conveniance. Equivalent to
      \code
      m_curvature_tessellation = false;
      m_threshhold = p;
      \endcode
      \param p value to which to assign to \ref m_threshhold
     */
    TessellationParams&
    curve_distance_tessellate(float p)
    {
      m_curvature_tessellation = false;
      m_threshhold = p;
      return *this;
    }

    /*!
      Set the value of \ref m_max_segments.
      \param v value to which to assign to \ref m_max_segments
     */
    TessellationParams&
    max_segments(unsigned int v)
    {
      m_max_segments = v;
      return *this;
    }

    /*!
      Specifies the meaning of \ref m_threshhold.
     */
    bool m_curvature_tessellation;

    /*!
      Meaning depends on \ref m_curvature_tessellation.
       - If m_curvature_tessellation is true, then represents the
         goal of how much curvature is to be between successive
         points of a tessellated edge. This value is crudely
         estimated via Simpson's rule. A value of PI / N for a circle
         represents tessellating the circle into N points.
       - If m_curvature_tessellation is false, then represents the
         goal of the maximum allowed distance between the
         line segment between from successive points of a
         tessellated edge and the sub-curve of the starting
         curve between those two points.
     */
    float m_threshhold;

    /*!
      Maximum number of segments to tessellate each
      PathContour::interpolator_base from each
      PathContour of a Path.
     */
    unsigned int m_max_segments;
  };

  /*!
    \brief
    Represents point of a tessellated path.
   */
  class point
  {
  public:
    /*!
      The position of the point.
     */
    vec2 m_p;

    /*!
      The derivative of the curve at the point.
     */
    vec2 m_p_t;

    /*!
      Gives the distance of the point from the start
      of the edge (i.e PathContour::interpolator_base)
      on which the point resides.
     */
    float m_distance_from_edge_start;

    /*!
      Gives the distance of the point from the start
      of the -contour- on which the point resides.
     */
    float m_distance_from_contour_start;

    /*!
      Gives the length of the edge (i.e.
      PathContour::interpolator_base) on which the
      point lies. This value is the same for all
      points along a fixed edge.
     */
    float m_edge_length;

    /*!
      Gives the length of the contour open on which
      the point lies. This value is the same for all
      points along a fixed contour.
     */
    float m_open_contour_length;

    /*!
      Gives the length of the contour closed on which
      the point lies. This value is the same for all
      points along a fixed contour.
     */
    float m_closed_contour_length;
  };

  /*!
    Ctor. Construct a TessellatedPath from a Path
    \param input source path to tessellate
    \param P parameters on how to tessellate the source Path
   */
  TessellatedPath(const Path &input, TessellationParams P);

  ~TessellatedPath();

  /*!
    Returns the tessellation parameters used to construct
    this TessellatedPath.
   */
  const TessellationParams&
  tessellation_parameters(void) const;

  /*!
    Returns the tessellation threshold achieved for
    distance between curve and sub-edges.
   */
  float
  effective_curve_distance_threshhold(void) const;

  /*!
    Returns the tessellation threshold achieved for
    the (approximated) curvature between successive
    points of the tessellation.
   */
  float
  effective_curvature_threshhold(void) const;

  /*!
    Returns the maximum number of segments any edge needed
   */
  unsigned int
  max_segments(void) const;

  /*!
    Returns all the point data
   */
  c_array<const point>
  point_data(void) const;

  /*!
    Returns the number of contours
   */
  unsigned int
  number_contours(void) const;

  /*!
    Returns the range into point_data()
    for the named contour. The contour data is a
    sequence of points specifing a line strip.
    Points that are shared between edges (an edge
    corresponds to a PathContour::interpolator_base) are
    replicated (because the derivatives are different).
    \param contour which path contour to query, must have
                   that 0 <= contour < number_contours()
   */
  range_type<unsigned int>
  contour_range(unsigned int contour) const;

  /*!
    Returns the range into point_data()
    for the named contour lacking the closing
    edge. The contour data is a
    sequence of points specifing a line strip.
    Points that are shared between edges (an edge
    corresponds to a PathContour::interpolator_base) are
    replicated (because the derivatives are different).
    \param contour which path contour to query, must have
                   that 0 <= contour < number_contours()
   */
  range_type<unsigned int>
  unclosed_contour_range(unsigned int contour) const;

  /*!
    Returns the point data of the named contour
    including the closing edge. The contour data is a
    sequence of points specifing a line strip.
    Points that are shared between edges (an edge
    corresponds to a PathContour::interpolator_base) are
    replicated (because the derivatives are different).
    Provided as a conveniance, equivalent to
    \code
    point_data().sub_array(contour_range(contour))
    \endcode
    \param contour which path contour to query, must have
                   that 0 <= contour < number_contours()
   */
  c_array<const point>
  contour_point_data(unsigned int contour) const;

  /*!
    Returns the point data of the named contour
    lacking the point data of the closing edge.
    The contour data is a sequence of points
    specifing a line strip. Points that are
    shared between edges (an edge corresponds to
    a PathContour::interpolator_base) are replicated
    (because the derivatives are different).
    Provided as a conveniance, equivalent to
    \code
    point_data().sub_array(unclosed_contour_range(contour))
    \endcode
    \param contour which path contour to query, must have
                   that 0 <= contour < number_contours()
   */
  c_array<const point>
  unclosed_contour_point_data(unsigned int contour) const;

  /*!
    Returns the number of edges for the named contour
    \param contour which path contour to query, must have
                   that 0 <= contour < number_contours()
   */
  unsigned int
  number_edges(unsigned int contour) const;

  /*!
    Returns the range into point_data(void)
    for the named edge of the named contour.
    The returned range does include the end
    point of the edge. The edge corresponds
    to the PathContour::interpolator_base
    Path::contour(contour)->interpolator(edge).
    \param contour which path contour to query, must have
                   that 0 <= contour < number_contours()
    \param edge which edge of the contour to query, must
                have that 0 <= edge < number_edges(contour)
   */
  range_type<unsigned int>
  edge_range(unsigned int contour, unsigned int edge) const;

  /*!
    Returns the point data of the named edge of the
    named contour, provided as a conveniance, equivalent
    to
    \code
    point_data().sub_array(edge_range(contour, edge))
    \endcode
    \param contour which path contour to query, must have
                   that 0 <= contour < number_contours()
    \param edge which edge of the contour to query, must
                have that 0 <= edge < number_edges(contour)
   */
  c_array<const point>
  edge_point_data(unsigned int contour, unsigned int edge) const;

  /*!
    Returns the minimum point of the bounding box of
    the tessellation.
   */
  vec2
  bounding_box_min(void) const;

  /*!
    Returns the maximum point of the bounding box of
    the tessellation.
   */
  vec2
  bounding_box_max(void) const;

  /*!
    Returns the dimensions of the bounding box
    of the tessellated path.
   */
  vec2
  bounding_box_size(void) const;

  /*!
    Returns this TessellatedPath stroked. The StrokedPath object
    is constructed lazily.
   */
  const reference_counted_ptr<const StrokedPath>&
  stroked(void) const;

  /*!
    Returns this TessellatedPath filled. The FilledPath object
    is constructed lazily.
   */
  const reference_counted_ptr<const FilledPath>&
  filled(void) const;

private:
  void *m_d;
};

/*! @} */

}
