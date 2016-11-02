#include "u_coords.h"
#include "fig.h"
#define SIGN(a1,a2) (a2 >= 0. ? fabs(a1) : -fabs(a1))

/* ---------------------------------------------------------------- *
 * Transform Latitude and Longitude Angles to Direction Cosines     *
 * ---------------------------------------------------------------- */

void LatLongToDcos3(const struct ltln *pa, struct vct3 *pe) {

   double cosphi;

   cosphi = cos(pa->lat);
   pe->di = cosphi * cos(pa->lng);
   pe->dj = cosphi * sin(pa->lng);
   pe->dk = sin(pa->lat);
   return;
   }

/* ---------------------------------------------------------------- *
 * Transform Direction Cosines to Latitude and Longitude Angles     *
 * ---------------------------------------------------------------- */

void Dcos3ToLatLong(const struct vct3 *pe, struct ltln *pa) {

   pa->lat = atan2(pe->dk, sqrt(pe->di * pe->di + pe->dj * pe->dj));
   pa->lng = atan2(pe->dj, pe->di);
   return;
   }

/* ---------------------------------------------------------------- *
 * Normalize a 3-D Direction Cosine Vector                          *
 * ---------------------------------------------------------------- */

void NormalizeDcos3(struct vct3 *vcc) {

   double d;

   d = 1.0 / sqrt(vcc->di * vcc->di +
                  vcc->dj * vcc->dj +
                  vcc->dk * vcc->dk);
   vcc->di *= d;
   vcc->dj *= d;
   vcc->dk *= d;
   return;
   }

/* ---------------------------------------------------------------- *
 * Normalize a 2-D Direction Cosine Vector                          *
 * ---------------------------------------------------------------- */

void NormalizeDcos2(struct vct2 *vcc) {

   double d;

   d = 1.0 / sqrt(vcc->di * vcc->di + vcc->dj * vcc->dj);
   vcc->di *= d;
   vcc->dj *= d;
   return;
   }

/* ---------------------------------------------------------------- *
 * Spherical Arc (Great Circle Distance) - First Approximation      *
 * ---------------------------------------------------------------- */

double ArcDist(const struct ltln *pea, const struct ltln *peb) {

   double AB, AC, BC;
   double sign_arg1, sign_arg2, tmp;

   AB = (90 - pea->lat) * DEG2RAD;
   AC = (90 - peb->lat) * DEG2RAD;
   tmp = pea->lng - peb->lng;

    sign_arg1 = SIGN(180.0, tmp+180.0);
    sign_arg2 = SIGN(180.0, 180.0-tmp); 
    tmp = tmp - sign_arg1 + sign_arg2;
   BC = fabs(tmp) * DEG2RAD;

   return (acos(cos(AB) * cos(AC) + sin(AB) * sin(AC) * cos(BC)));
   }

/* ---------------------------------------------------------------- *
 * Direct Stereographic Projection (Map, Sphere to Plane)           *
 * ---------------------------------------------------------------- */

void MapStereo(const struct vct3 *p0,
               const struct vct3 *pe, struct plpt *pw) {
   struct vct3 prln;
   double      t, am, bm, ap, bp, cp, xi, yi, zi;
/* ---------------------------------------------------------------- */

/* Find tangency point relative values.                             */

   cp = sqrt(p0->di * p0->di + p0->dj * p0->dj);
   am = -(p0->dj / cp);
   bm = p0->di / cp;
   ap = -(p0->dk * bm);
   bp = p0->dk * am;

/* Intersection of the projection line and the intersection plane.  */

   prln.di = -(p0->di + pe->di);
   prln.dj = -(p0->dj + pe->dj);
   prln.dk = -(p0->dk + pe->dk);

   NormalizeDcos3(&prln);

   t = -((p0->di * pe->di + p0->dj * pe->dj + p0->dk * pe->dk - 1.0) /
         (p0->di * prln.di + p0->dj * prln.dj + p0->dk * prln.dk));

   xi = pe->di + prln.di * t;
   yi = pe->dj + prln.dj * t;
   zi = pe->dk + prln.dk * t;

/* Stereographic plane coordinates are the oriented distances from
   the intersection point to the meridian and prime vertical plane. */

   pw->est = am * xi + bm * yi;
   pw->nrt = ap * xi + bp * yi + cp * zi;
   return;
   }

/* ---------------------------------------------------------------- *
 * Inverse Stereographic Projection (Un-Map, Plane to Sphere)       *
 * ---------------------------------------------------------------- */

void UnMapStereo(const struct vct3 *p0,
                 const struct plpt *pw, struct vct3 *pe) {
   struct vct3   prln;
   double        gcx, am, bm, ap, bp, cp, cpsq;
   double        xe, ye, ze, xc, yc, zc, lymx, lxmy, root, t;
/* ---------------------------------------------------------------- */

/* Find the sphere/plane tangency point values: ap, bp, cp are
   components of the "North" vector, and am, bm of "East" vector
   in this point. The "East" vector has no "Z" axis component.      */

   gcx = sqrt(pw->est * pw->est + pw->nrt * pw->nrt);

   cpsq = p0->di * p0->di + p0->dj * p0->dj;
   cp = sqrt(cpsq);
   am = -(p0->dj / cp);
   bm = p0->di / cp;

   ap = -(p0->dk * bm);
   bp = p0->dk * am;

/* Find Cartesian coordinates of the projection center (xc,yc,zc)
   and the projected point in the plane of projection (xe,ye,ze).   */

   xc = -p0->di;
   yc = -p0->dj;
   zc = -p0->dk;

   xe = -xc + ap * pw->nrt + am * pw->est;
   ye = -yc + bp * pw->nrt + bm * pw->est;
   ze = -zc + cp * pw->nrt;

/* Find the intersection of ptc-pte line and the sphere.
   Solution requires solving a quadratic in t, the line parameter.  */

   prln.di = -gcx;
   prln.dj = -2.0;
   NormalizeDcos2((struct vct2 *)&prln);
   lymx = prln.dj * gcx - prln.di;
   lxmy = -(prln.di * gcx + prln.dj);
   root = sqrt(1.0 - (lymx * lymx));

   t = lxmy - root;  /* Find the closer of the two quadratic roots. */

/* Substitute the parameter in the parametric line equations.       */

   prln.di = xc - xe;
   prln.dj = yc - ye;
   prln.dk = zc - ze;
   NormalizeDcos3(&prln);
   pe->di = xe + t * prln.di;
   pe->dj = ye + t * prln.dj;
   pe->dk = ze + t * prln.dk;
   NormalizeDcos3(pe);
   return;
   }

/* ---------------------------------------------------------------- *
 * Initialize Projection Plane / Display Translation and Scaling    *
 * ---------------------------------------------------------------- */

void SetPlaneDisplay(double *xfmArray,
                     const struct plpt *w1, const struct plpt *w2,
                     const struct dpxl *d1, const struct dpxl *d2) {

   double   dx, dy, du, dv;

   dx = (w2->est) - (w1->est);
   dy = (w2->nrt) - (w1->nrt);
   du = (double)((d2->x) - (d1->x));
   dv = (double)((d2->y) - (d1->y));
   xfmArray[0] = dx / du;
   xfmArray[1] = dy / dv;
   xfmArray[2] = du / dx;
   xfmArray[3] = dv / dy;
   xfmArray[4] = w1->est - xfmArray[0] * ((double)d1->x + 0.5);
   xfmArray[5] = w1->nrt - xfmArray[1] * ((double)d1->y + 0.5);
   xfmArray[6] = ((double)d1->x + 0.5) - xfmArray[2] * w1->est;
   xfmArray[7] = ((double)d1->y + 0.5) - xfmArray[3] * w1->nrt;
   return;
   }

/* ---------------------------------------------------------------- *
 * Translate/Scale Point from a Projection Plane to the Display     *
 * ---------------------------------------------------------------- */

void PlaneToDisplay(const double *xfmArray,
                    const struct plpt *w, struct dpxl *d) {

   d->x = (short)(xfmArray[6] + xfmArray[2] * w->est);
   d->y = (short)(xfmArray[7] + xfmArray[3] * w->nrt);
   return;
   }

/* ---------------------------------------------------------------- *
 * Translate/Scale Point from the Display to a Projection Plane     *
 * ---------------------------------------------------------------- */

void DisplayToPlane(const double *xfmArray,
                    const struct dpxl *d, struct plpt *w) {

   w->est = xfmArray[4] + xfmArray[0] * ((double)d->x + 0.5);
   w->nrt = xfmArray[5] + xfmArray[1] * ((double)d->y + 0.5);
   return;
   }
