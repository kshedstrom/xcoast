/* ---------------------------------------------------------------- *
 * ALGEBRA functions: A sampling of geometronical vector algebra    *
 * functions and their supporting manifest constants and structure  *
 * declarations.                                                    *
 *                                                                  *
 * The following code is derived from similar functions which are   *
 * a small part of the Hipparchus Library. For simplicity, it lacks *
 * the "fuzz control" and other programming elements of practical   *
 * numerical significance.                                          *
 *                                                                  *
 * Programmer: Hrvoje Lukatela, September 1992.                     *
 * Geodyssey Limited, Calgary - (403) 234 9848, fax: (403) 266 7117 *
 ------------------------------------------------------------------ */

#include <math.h>

static const double PI = 3.14159265358979324;
static const double DEG2RAD = .017453292519943;    /* degrees to radians... */
static const double RAD2DEG = 57.2957795130823;    /* ... and vice versa */
static const double REarth = 6.3708e6;             /* radius of Earth */

extern double udeg;

struct plpt {              /* point in a Cartesian projection plane */
   double est;
   double nrt;
   };

struct lclpt {                        /* local (object) coordinates */
   short est;
   short nrt;
   };

struct dpxl {                         /* display screen coordinates */
   short x;
   short y;
   };

struct ltln {                  /* point latitude-longitude, radians */
   double lat;
   double lng;
   };

struct vct3 {                /* 3-D vector; x,y,z direction cosines */
   double di;
   double dj;
   double dk;
   };

struct vct2 {                   /* as above, in plane, internal use */
   double di;
   double dj;
   };

struct indexRec {             /* line segment database index record */
   struct vct3 center;               /* nominal object center point */
   double      radius;                     /* in radian arc measure */
   long        fileOffset;    /* offset in the coordinate data file */
   short       vertexCount;         /* count of coordinate vertices */
   short       segmentId;          /* for possible application use? */
   };

extern void LatLongToDcos3(const struct ltln *pa, struct vct3 *pe);
extern void Dcos3ToLatLong(const struct vct3 *pe, struct ltln *pa);
extern void NormalizeDcos3(struct vct3 *vcc);
extern void NormalizeDcos2(struct vct2 *vcc);
extern double ArcDist(const struct ltln *pea, const struct ltln *peb);
extern void MapStereo(const struct vct3 *p0,
                      const struct vct3 *pe, struct plpt *pw);
extern void UnMapStereo(const struct vct3 *p0,
                      const struct plpt *pw, struct vct3 *pe);
extern void SetPlaneDisplay(double *xfmArray,
                      const struct plpt *w1, const struct plpt *w2,
                      const struct dpxl *d1, const struct dpxl *d2);
extern void PlaneToDisplay(const double *xfmArray,
                      const struct plpt *w, struct dpxl *d);
extern void DisplayToPlane(const double *xfmArray,
                      const struct dpxl *d, struct plpt *w);
