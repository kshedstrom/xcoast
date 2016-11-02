#include <ncarg/ncargC.h>
#include <ncarg/gks.h>

#define IWTYPE 1
#define WKID   1

int main()
{
    char            ch[3];
    float	    rlat1, rlon1, rlat2, rlon2;
    float	    plat, plon, rota;
    float	    uu, vv, lon, lat;


    scanf("%s\n", ch);
    scanf("%f  %f  %f", &plat, &plon, &rota);
    scanf("%f  %f  %f  %f", &rlat1, &rlon1, &rlat2, &rlon2);

/*
 * open GKS
 */
    gopen_gks("stdout",0);
    gopen_ws(WKID, NULL, IWTYPE);
    gactivate_ws(WKID);

/*
 * Initialize the mapping
 */

    c_mapstc ("OU","CO");
    c_maproj (ch, plat, plon, rota);
    c_mapset("CO", &rlat1, &rlon1, &rlat2, &rlon2);
    c_mapdrw();
    while (scanf("%f %f", &uu, &vv) != EOF) {
	if (uu != 1000.) {
	    c_maptri(uu, vv, &lat, &lon);
	    printf("%f  %f\n", lat, lon);
	} else {
	    printf("1000. 1000.\n");
	}
    }

/*
 * Close GKS.
 */
    gdeactivate_ws(WKID);
    gclose_ws(WKID);
    gclose_gks();
}
