// Copyright (c) 2025 OPEN CASCADE SAS
//
// This file is part of Open CASCADE Technology software library.
//
// This library is free software; you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License version 2.1 as published
// by the Free Software Foundation, with special exception defined in the file
// OCCT_LGPL_EXCEPTION.txt. Consult the file LICENSE_LGPL_21.txt included in OCCT
// distribution for complete text of the license and disclaimer of any warranty.
//
// Alternatively, this file may be used under the terms of Open CASCADE
// commercial license or contractual agreement.

#include <Geom_BSplineCurve.hxx>
#include <Geom_BSplineSurface.hxx>
#include <Geom_Circle.hxx>
#include <Geom_Line.hxx>
#include <Geom_Plane.hxx>
#include <Geom_RectangularTrimmedSurface.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <GeomConvert.hxx>
#include <gp.hxx>
#include <gp_Ax2.hxx>
#include <gp_Dir.hxx>
#include <gp_Pnt.hxx>
#include <NCollection_Array1.hxx>
#include <NCollection_HArray1.hxx>
#include <Precision.hxx>

#include <gtest/gtest.h>

TEST(GeomConvertTest, CircleToBSpline)
{
  occ::handle<Geom_Circle> aCircle =
    new Geom_Circle(gp_Ax2(gp_Pnt(0.0, 0.0, 0.0), gp_Dir(0.0, 0.0, 1.0)), 5.0);

  occ::handle<Geom_BSplineCurve> aBSpline = GeomConvert::CurveToBSplineCurve(aCircle);

  ASSERT_FALSE(aBSpline.IsNull());

  gp_Pnt aOrigPnt;
  aCircle->D0(0.0, aOrigPnt);
  gp_Pnt aBSplinePnt;
  aBSpline->D0(0.0, aBSplinePnt);

  EXPECT_NEAR(aOrigPnt.Distance(aBSplinePnt), 0.0, Precision::Confusion());
}

TEST(GeomConvertTest, LineToBSpline)
{
  occ::handle<Geom_Line> aLine = new Geom_Line(gp_Pnt(0.0, 0.0, 0.0), gp_Dir(1.0, 0.0, 0.0));
  occ::handle<Geom_TrimmedCurve> aTrimmed = new Geom_TrimmedCurve(aLine, 0.0, 10.0);

  occ::handle<Geom_BSplineCurve> aBSpline = GeomConvert::CurveToBSplineCurve(aTrimmed);

  ASSERT_FALSE(aBSpline.IsNull());

  gp_Pnt aPnt;
  aBSpline->D0(5.0, aPnt);
  const gp_Pnt anExpectedPnt(5.0, 0.0, 0.0);

  EXPECT_NEAR(aPnt.Distance(anExpectedPnt), 0.0, Precision::Confusion());
}

TEST(GeomConvertTest, PlaneToBSplineSurface)
{
  occ::handle<Geom_Plane>                     aPlane = new Geom_Plane(gp::XOY());
  occ::handle<Geom_RectangularTrimmedSurface> aTrimmed =
    new Geom_RectangularTrimmedSurface(aPlane, 0.0, 10.0, 0.0, 10.0);

  occ::handle<Geom_BSplineSurface> aBSurf = GeomConvert::SurfaceToBSplineSurface(aTrimmed);

  ASSERT_FALSE(aBSurf.IsNull());

  gp_Pnt aPnt;
  aBSurf->D0(5.0, 5.0, aPnt);
  const gp_Pnt anExpectedPnt(5.0, 5.0, 0.0);

  EXPECT_NEAR(aPnt.Distance(anExpectedPnt), 0.0, Precision::Confusion());
}

// A closed group of curves must be concatenated starting from its first curve: the last curve
// matches both ends of the accumulated curve and has to be appended, not prepended.
TEST(GeomConvertTest, ConcatG1_ClosedGroupKeepsStart)
{
  occ::handle<Geom_Circle> aCircle =
    new Geom_Circle(gp_Ax2(gp_Pnt(0.0, 0.0, 0.0), gp_Dir(0.0, 0.0, 1.0)), 5.0);
  const double aBounds[4] = {0.5, 2.0, 4.5, 0.5 + 2.0 * M_PI};

  NCollection_Array1<occ::handle<Geom_BSplineCurve>> aCurves(0, 2);
  NCollection_Array1<double>                         aTolers(0, 1);
  for (int i = 0; i < 3; ++i)
  {
    aCurves(i) =
      GeomConvert::CurveToBSplineCurve(new Geom_TrimmedCurve(aCircle, aBounds[i], aBounds[i + 1]));
  }
  aTolers.Init(Precision::Confusion());
  const gp_Pnt aStart = aCurves(0)->StartPoint();

  occ::handle<NCollection_HArray1<occ::handle<Geom_BSplineCurve>>> aResult;
  bool                                                             isClosed = false;
  GeomConvert::ConcatG1(aCurves, aTolers, aResult, isClosed, Precision::Confusion());

  ASSERT_FALSE(aResult.IsNull());
  ASSERT_EQ(aResult->Length(), 1);
  const occ::handle<Geom_BSplineCurve>& aConcat = aResult->Value(aResult->Lower());
  EXPECT_NEAR(aConcat->StartPoint().Distance(aStart), 0.0, Precision::Confusion());
  EXPECT_NEAR(aConcat->EndPoint().Distance(aStart), 0.0, Precision::Confusion());
}
