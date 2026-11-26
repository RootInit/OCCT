// Copyright (c) 2026 OPEN CASCADE SAS
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

#include <Geom2d_BSplineCurve.hxx>
#include <Geom2d_Circle.hxx>
#include <Geom2d_TrimmedCurve.hxx>
#include <Geom2dConvert.hxx>
#include <gp_Ax2d.hxx>
#include <gp_Dir2d.hxx>
#include <gp_Pnt2d.hxx>
#include <NCollection_Array1.hxx>
#include <NCollection_HArray1.hxx>
#include <Precision.hxx>

#include <gtest/gtest.h>

namespace
{
//! Fills the arrays with three arcs that together make a full circle starting at parameter 0.5.
void makeClosedGroup(NCollection_Array1<occ::handle<Geom2d_BSplineCurve>>& theCurves,
                     NCollection_Array1<double>&                           theTolers)
{
  occ::handle<Geom2d_Circle> aCircle =
    new Geom2d_Circle(gp_Ax2d(gp_Pnt2d(0.0, 0.0), gp_Dir2d(1.0, 0.0)), 5.0);
  const double aBounds[4] = {0.5, 2.0, 4.5, 0.5 + 2.0 * M_PI};
  for (int i = 0; i < 3; ++i)
  {
    theCurves(i) = Geom2dConvert::CurveToBSplineCurve(
      new Geom2d_TrimmedCurve(aCircle, aBounds[i], aBounds[i + 1]));
  }
  theTolers.Init(Precision::Confusion());
}
} // namespace

// A closed group of curves must be concatenated starting from its first curve: the last curve
// matches both ends of the accumulated curve and has to be appended, not prepended.
TEST(Geom2dConvertTest, ConcatG1_ClosedGroupKeepsStart)
{
  NCollection_Array1<occ::handle<Geom2d_BSplineCurve>> aCurves(0, 2);
  NCollection_Array1<double>                           aTolers(0, 1);
  makeClosedGroup(aCurves, aTolers);
  const gp_Pnt2d aStart = aCurves(0)->StartPoint();

  occ::handle<NCollection_HArray1<occ::handle<Geom2d_BSplineCurve>>> aResult;
  bool                                                               isClosed = false;
  Geom2dConvert::ConcatG1(aCurves, aTolers, aResult, isClosed, Precision::Confusion());

  ASSERT_FALSE(aResult.IsNull());
  ASSERT_EQ(aResult->Length(), 1);
  const occ::handle<Geom2d_BSplineCurve>& aConcat = aResult->Value(aResult->Lower());
  EXPECT_NEAR(aConcat->StartPoint().Distance(aStart), 0.0, Precision::Confusion());
  EXPECT_NEAR(aConcat->EndPoint().Distance(aStart), 0.0, Precision::Confusion());
}

TEST(Geom2dConvertTest, ConcatC1_ClosedGroupKeepsStart)
{
  NCollection_Array1<occ::handle<Geom2d_BSplineCurve>> aCurves(0, 2);
  NCollection_Array1<double>                           aTolers(0, 1);
  makeClosedGroup(aCurves, aTolers);
  const gp_Pnt2d aStart = aCurves(0)->StartPoint();

  occ::handle<NCollection_HArray1<int>>                              anIndices;
  occ::handle<NCollection_HArray1<occ::handle<Geom2d_BSplineCurve>>> aResult;
  bool                                                               isClosed = false;
  Geom2dConvert::ConcatC1(aCurves, aTolers, anIndices, aResult, isClosed, Precision::Confusion());

  ASSERT_FALSE(aResult.IsNull());
  ASSERT_EQ(aResult->Length(), 1);
  const occ::handle<Geom2d_BSplineCurve>& aConcat = aResult->Value(aResult->Lower());
  EXPECT_NEAR(aConcat->StartPoint().Distance(aStart), 0.0, Precision::Confusion());
  EXPECT_NEAR(aConcat->EndPoint().Distance(aStart), 0.0, Precision::Confusion());
}
