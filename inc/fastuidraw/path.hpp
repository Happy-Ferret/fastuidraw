/*!
 * \file path.hpp
 * \brief file path.hpp
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
#include <fastuidraw/tessellated_path.hpp>

namespace fastuidraw  {

/*!\addtogroup Core
  @{
 */

/*!
  An PathContour represents a single contour within
  a Path.
 */
class PathContour:
    public reference_counted<PathContour>::non_concurrent
{
public:

  /*!
    Base class to describe how to interpolate from one
    point of a PathContour to the next, i.e. describes
    the shape of an edge.
   */
  class interpolator_base:
    public reference_counted<interpolator_base>::non_concurrent
  {
  public:
    /*!
      Ctor.
      \param prev interpolator to edge that ends at start of edge
                  of this interpolator
      \param end end point of the edge of this interpolator
     */
    interpolator_base(const const_handle &prev, const vec2 &end);

    virtual
    ~interpolator_base();

    /*!
      Returns the interpolator previous to this interpolator_base
      within the PathContour that this object resides.
     */
    const_handle
    prev_interpolator(void) const;

    /*!
      Returns the starting point of this interpolator.
     */
    const vec2&
    start_pt(void) const;

    /*!
      Returns the ending point of this interpolator
     */
    const vec2&
    end_pt(void) const;

    /*!
      To be implemented by a derived class to produce the tessellation
      from start_pt() to end_pt(). The routine should
      include BOTH start_pt() and end_pt() in the result.
      Assignments to the field TessellatedPath::point::m_distance_from_outline_start
      will be ignored, but all other fields of TessellatedPath::point
      must be assigned values. Return the number of points actually added.

      \param tess_params tessellation parameters
      \param out_data location to which to write the edge tessellated
     */
    virtual
    unsigned int
    produce_tessellation(const TessellatedPath::TessellationParams &tess_params,
                         c_array<TessellatedPath::point> out_data) const = 0;

  private:
    friend class PathContour;
    void *m_d;
  };

  /*!
    A flat interpolator represents a flat edge.
   */
  class flat:public interpolator_base
  {
  public:
    /*!
      Ctor.
      \param prev interpolator to edge that ends at start of edge
                  of this interpolator
      \param end end point of the edge of this interpolator
     */
    flat(const const_handle &prev, const vec2 &end):
      interpolator_base(prev, end)
    {}

    virtual
    unsigned int
    produce_tessellation(const TessellatedPath::TessellationParams &tess_params,
                         c_array<TessellatedPath::point> out_data) const;
  };

  /*!
    Interpolator generic implements tessellation by recursion
    and relying on analytic derivative provided by derived
    class.
   */
  class interpolator_generic:public interpolator_base
  {
  public:
    /*!
      Ctor.
      \param prev interpolator to edge that ends at start of edge
                  of this interpolator
      \param end end point of the edge of this interpolator
     */
    interpolator_generic(const const_handle &prev, const vec2 &end):
      interpolator_base(prev, end)
    {}

    virtual
    unsigned int
    produce_tessellation(const TessellatedPath::TessellationParams &tess_params,
                         c_array<TessellatedPath::point> out_data) const;

    /*!
      To be implemented by a derived class to compute datum of the curve
      at a time t (0 <= t <= 1).
      \param in_t (input) argument fed to parameterization of which this
                  interpolator_base represents with in_t = 0.0
                  indicating the start of the curve and in_t = 1.0
                  the end of the curve
      \param outp (output) reference to place position of the curve
      \param outp_t (output) reference to place the first derivative
      \param outp_tt (output) reference to place the second derivative
     */
    virtual
    void
    compute(float in_t, vec2 &outp, vec2 &outp_t, vec2 &outp_tt) const = 0;

  private:
  };

  /*!
    Derived class of interpolator_base to indicate a Bezier curve.
    Supports Bezier curves of _any_ degree.
   */
  class bezier:public interpolator_generic
  {
  public:
    /*!
      Ctor. One control point, thus interpolation is a quadratic curve.
      \param start start of curve
      \param ct control point
      \param end end of curve
     */
    bezier(const const_handle &start, const vec2 &ct, const vec2 &end);

    /*!
      Ctor. Two control points, thus interpolation is a cubic curve.
      \param start start of curve
      \param ct1 1st control point
      \param ct2 2nd control point
      \param end end point of curve
     */
    bezier(const const_handle &start, const vec2 &ct1,
           const vec2 &ct2, const vec2 &end);

    /*!
                  const_c_array<vec2> control_pts,
                  const vec2 &)
      Ctor. Iterator range defines the control points of the bezier curve.
      \param start start of curve
      \param control_pts control points
      \param end end point of curve
     */
    bezier(const const_handle &start,
           const_c_array<vec2> control_pts,
           const vec2 &end);

    virtual
    ~bezier();

    virtual
    void
    compute(float in_t, vec2 &outp, vec2 &outp_t, vec2 &outp_tt) const;

  private:
    void *m_d;
  };

  /*!
    An arc is for connecting one point to the next via an
    arc of a circle.
   */
  class arc:public interpolator_base
  {
  public:

    /*!
      Ctor.
      \param start start of curve
      \param angle The angle of the arc in radians, the value must not
                   be a multiple of 2*M_PI. Assuming a coordinate system
                   where y-increases vertically and x-increases to the right,
                   a positive value indicates to have the arc go counter-clockwise,
                   a negative angle for the arc to go clockwise.
      \param end end of curve
     */
    arc(const const_handle &start, float angle, const vec2 &end);

    ~arc();

    unsigned int
    produce_tessellation(const TessellatedPath::TessellationParams &tess_params,
                         c_array<TessellatedPath::point> out_data) const;

  private:
    void *m_d;
  };

  /*!
    Ctor.
   */
  explicit
  PathContour(void);

  ~PathContour();

  /*!
    Start the PathContour, may only be called once in the lifetime
    of a PathContour() and must be called before adding points
    (to_point()), adding control points (add_control_point()),
    adding arcs (to_arc()), adding generic interpolator (
    to_generic()) or ending the contour (end(), end_generic()).
   */
  void
  start(const vec2 &pt);

  /*!
    End the current edge.
    \param pt point location of end of edge (and thus start of new edge)
   */
  void
  to_point(const vec2 &pt);

  /*!
    Add a control point. Will fail if end() was called
    \param pt location of new control point
   */
  void
  add_control_point(const vec2 &pt);

  /*!
    Will fail if end() was called of if add_control_point() has been
    called more recently than to_point() or if interpolator_base::prev_interpolator()
    of the passed interpolator_base is not the same as prev_interpolator().
    \param p interpolator describing edge
   */
  void
  to_generic(const interpolator_base::const_handle &p);

  /*!
    Will fail if end() was called of if add_control_point() has been
    called more recently than to_point().
    \param angle angle of arc in radians
    \param pt point where arc ends (and next edge starts)
   */
  void
  to_arc(float angle, const vec2 &pt);

  /*!
    Ends with the passed interpolator_base. The interpolator
    must interpolate to the start point of the PathContour
    \param h interpolator describing the closing edge
   */
  void
  end_generic(interpolator_base::const_handle h);

  /*!
    Ends with the Bezier curve defined by the current
    control points added by add_control_point().
   */
  void
  end(void);

  /*!
    Ends with an arc.
    \param angle angle of arc in radians
   */
  void
  end_arc(float angle);

  /*!
    Returns the last interpolator added to this PathContour.
    You MUST use this interpolator in the ctor of
    interpolator_base for interpolator passed to
    to_generic() and end().
   */
  const interpolator_base::const_handle&
  prev_interpolator(void);

  /*!
    Returns true if the current path has ended
   */
  bool
  ended(void) const;

  /*!
    Return the I'th point of this PathContour.
    For I = 0, returns the value passed to start().
    \param I index of point.
   */
  const vec2&
  point(unsigned int I) const;

  /*!
    Returns the number of points of this PathContour.
   */
  unsigned int
  number_points(void) const;

  /*!
    Returns the interpolator of this PathContour
    that interpolates from the I'th point to the
    (I+1)'th point. If I == number_points() - 1,
    then returns the interpolator from the last
    point to the first point.
   */
  const interpolator_base::const_handle&
  interpolator(unsigned int I) const;

private:
  void *m_d;
};

/*!
  A Path represents a collection of PathContour
  objects.
 */
class Path
{
public:
  /*!
    Class that wraps a vec2 to mark a point
    as a control point for a Bezier curve
   */
  class control_point
  {
  public:
    /*!
      Ctor
      \param pt value to which to set m_location
     */
    explicit
    control_point(const vec2 &pt):
      m_location(pt)
    {}

    /*!
      Ctor
      \param x value to which to set m_location.x()
      \param y value to which to set m_location.y()
     */
    control_point(float x, float y):
      m_location(x,y)
    {}

    /*!
      Position of control point
     */
    vec2 m_location;
  };

  /*!
    Wraps the data to specify an arc
   */
  class arc
  {
  public:
    /*!
      Ctor
      \param angle angle of arc in radians
      \param pt point to which to arc
     */
    arc(float angle, const vec2 &pt):
      m_angle(angle), m_pt(pt)
    {}

    /*!
      Angle of arc in radians
     */
    float m_angle;

    /*!
      End point of arc
     */
    vec2 m_pt;
  };

  /*!
    Tag class to mark the end of an outline
   */
  class end
  {};

  /*!
    Tag class to mark the end of an outline with an arc
   */
  class end_arc
  {
  public:
    /*!
      Ctor
      \param angle angle of arc in radians
     */
    explicit
    end_arc(float angle):
      m_angle(angle)
    {}

    /*!
      Angle of arc in radians
     */
    float m_angle;
  };

  /*!
    Ctor.
    \param tess_params the tessellation parameters for when the Path
                       is tessellated into a TessellatedPath,
                       see tessellation().
   */
  explicit
  Path(const TessellatedPath::TessellationParams &tess_params = TessellatedPath::TessellationParams());

  /*!
    Ctor.
    \param obj Path from which to copy path data
    \param tess_param tessellation parameters to use for new
                      Path.
   */
  Path(const Path &obj, const TessellatedPath::TessellationParams &tess_param);

  ~Path();

  /*!
    Create an arc but specify the angle in degrees.
    \param angle angle of arc in degrees
    \param pt point to which to arc
   */
  static
  arc
  arc_degrees(float angle, const vec2 &pt)
  {
    return arc(angle*float(M_PI)/180.0f, pt);
  }

  /*!
    Create an end_arc but specify the angle in degrees.
    \param angle angle or arc in degrees
   */
  static
  end_arc
  end_arc_degrees(float angle)
  {
    return end_arc(angle*float(M_PI)/180.0f);
  }

  /*!
    Operator overload to add a point of the current
    outline in the Path.
    \param pt point to add
   */
  Path&
  operator<<(const vec2 &pt);

  /*!
    Operator overload to add a control point of the current
    outline in the Path.
    \param pt control point to add
   */
  Path&
  operator<<(const control_point &pt);

  /*!
    Operator overload to add an arc to the current outline
    in the Path.
    \param a arc to add
   */
  Path&
  operator<<(const arc &a);

  /*!
    Operator overload to end the current outline
   */
  Path&
  operator<<(end);

  /*!
    Operator overload to end the current outline
    \param a specifies the angle of the arc for closing
             the current outline
   */
  Path&
  operator<<(end_arc a);

  /*!
    Append a line to the current outline.
    \param pt point to which the line goes
   */
  Path&
  line_to(const vec2 &pt);

  /*!
    Append a quadratic Bezier curve to the current outline.
    \param ct control point of the quadratic Bezier curve
    \param pt point to which the quadratic Bezier curve goes
   */
  Path&
  quadratic_to(const vec2 &ct, const vec2 &pt);

  /*!
    Append a cubic Bezier curve to the current outline.
    \param ct1 first control point of the cubic Bezier curve
    \param ct2 second control point of the cubic Bezier curve
    \param pt point to which the cubic Bezier curve goes
   */
  Path&
  cubic_to(const vec2 &ct1, const vec2 &ct2, const vec2 &pt);

  /*!
    Append an arc curve to the current outline.
    \param angle gives the angle of the arc in radians. For a coordinate system
                 where y increases upwards and x increases to the right, a positive
                 value indicates counter-clockwise and a negative value indicates
                 clockwise
    \param pt point to which the arc curve goes
   */
  Path&
  arc_to(float angle, const vec2 &pt);

  /*!
    Returns the last interpolator added to this the current
    outline of this Path. When creating custom
    interpolator to be added with custom_to(),
    You MUST use this interpolator in the ctor of
    interpolator_base.
   */
  const PathContour::interpolator_base::const_handle&
  prev_interpolator(void);

  /*!
    Add a custom interpolator. Use prev_interpolator()
    to get the last interpolator of the current outline.
   */
  Path&
  custom_to(const PathContour::interpolator_base::const_handle &p);

  /*!
    Begin a new outline
    \param pt point at which the outline begins
   */
  Path&
  move(const vec2 &pt);

  /*!
    End the current outline in an arc and begin an new outline
    \param angle gives the angle of the arc in radians. For a coordinate system
                 where y increases upwards and x increases to the right, a positive
                 value indicates counter-clockwise and a negative value indicates
                 clockwise
    \param pt point at which the outline begins
   */
  Path&
  arc_move(float angle, const vec2 &pt);

  /*!
    End the current outline in an arc
    \param angle gives the angle of the arc in radians. For a coordinate system
                 where y increases upwards and x increases to the right, a positive
                 value indicates counter-clockwise and a negative value indicates
                 clockwise
   */
  Path&
  arc_end(float angle);

  /*!
    End the current outline in a quadratic Bezier curve and begin an new outline
    \param ct control point of the quadratic Bezier curve
    \param pt point at which the outline begins
   */
  Path&
  quadratic_move(const vec2 &ct, const vec2 &pt);

  /*!
    End the current outline in a quadratic Bezier curve
    \param ct control point of the quadratic Bezier curve
   */
  Path&
  quadratic_end(const vec2 &ct);

  /*!
    End the current outline in a cubic Bezier curve and begin an new outline
    \param ct1 first control point of the cubic Bezier curve
    \param ct2 second control point of the cubic Bezier curve
    \param pt point at which the outline begins
   */
  Path&
  cubic_move(const vec2 &ct1, const vec2 &ct2, const vec2 &pt);

  /*!
    End the current outline in a cubic Bezier curve
    \param ct1 first control point of the cubic Bezier curve
    \param ct2 second control point of the cubic Bezier curve
   */
  Path&
  cubic_end(const vec2 &ct1, const vec2 &ct2);

  /*!
    Use a custom interpolator to end the current outline and begin a new outline
    Use prev_interpolator() to get the last interpolator
    of the current outline.
    \param p custom interpolator
    \param pt point at which the outline begins
   */
  Path&
  custom_move(const PathContour::interpolator_base::const_handle &p, const vec2 &pt);

  /*!
    Use a custom interpolator to end the current outline
    Use prev_interpolator() to get the last interpolator
    of the current outline.
    \param p custom interpolator
   */
  Path&
  custom_end(const PathContour::interpolator_base::const_handle &p);

  /*!
    Returns the number of outlines of the Path.
   */
  unsigned int
  number_outlines(void) const;

  /*!
    Returns the named outline
    \param i index of outline to fetch (0 <= i < number_outlines())
   */
  PathContour::const_handle
  outline(unsigned int i) const;

  /*!
    Returns the tessellation parameters used
    to construct the TessellatedPath
    returned by tessellation()
   */
  const TessellatedPath::TessellationParams&
  tessellation_params(void) const;

  /*!
    Set the tessellation parameters used
    to construct the TessellatedPath
    returned by tessellation().
    \param p new tessellation parameters
   */
  void
  tessellation_params(const TessellatedPath::TessellationParams &p);

  /*!
    Return the tessellation of this Path. The
    TessellatedPath is constructed lazily. Additionally,
    if this Path changes its geometry or
    tessellation parameters, then a new TessellatedPath
    will be contructed on the next call to
    tessellation().
   */
  const TessellatedPath::const_handle&
  tessellation(void) const;

private:
  void *m_d;
};

/*! @} */

}