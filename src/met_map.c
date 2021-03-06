/*
  This file is part of MPTRAC.
  
  MPTRAC is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
  
  MPTRAC is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with MPTRAC. If not, see <http://www.gnu.org/licenses/>.
  
  Copyright (C) 2013-2019 Forschungszentrum Juelich GmbH
*/

/*! 
  \file
  Extract map from meteorological data.
*/

#include "libtrac.h"

/* ------------------------------------------------------------
   Dimensions...
   ------------------------------------------------------------ */

/*! Maximum number of longitudes. */
#define NX 1441

/*! Maximum number of latitudes. */
#define NY 721

/* ------------------------------------------------------------
   Main...
   ------------------------------------------------------------ */

int main(
  int argc,
  char *argv[]) {

  ctl_t ctl;

  met_t *met;

  FILE *out;

  static double timem[NX][NY], p0, ps, psm[NX][NY], pt, ptm[NX][NY], t,
    tm[NX][NY], u, um[NX][NY], v, vm[NX][NY], w, wm[NX][NY], h2o,
    h2om[NX][NY], h2ot, h2otm[NX][NY], o3, o3m[NX][NY],
    lwc, lwcm[NX][NY], iwc, iwcm[NX][NY], z, zm[NX][NY], pv,
    pvm[NX][NY], zt, ztm[NX][NY], tt, ttm[NX][NY],
    pc, pcm[NX][NY], cl, clm[NX][NY], lon, lon0, lon1, lons[NX],
    dlon, lat, lat0, lat1, lats[NY], dlat, cw[3];

  static int i, ix, iy, np[NX][NY], nx, ny, ci[3];

  /* Allocate... */
  ALLOC(met, met_t, 1);

  /* Check arguments... */
  if (argc < 4)
    ERRMSG("Give parameters: <ctl> <map.tab> <met0> [ <met1> ... ]");

  /* Read control parameters... */
  read_ctl(argv[1], argc, argv, &ctl);
  p0 = P(scan_ctl(argv[1], argc, argv, "MAP_Z0", -1, "10", NULL));
  lon0 = scan_ctl(argv[1], argc, argv, "MAP_LON0", -1, "-180", NULL);
  lon1 = scan_ctl(argv[1], argc, argv, "MAP_LON1", -1, "180", NULL);
  dlon = scan_ctl(argv[1], argc, argv, "MAP_DLON", -1, "-999", NULL);
  lat0 = scan_ctl(argv[1], argc, argv, "MAP_LAT0", -1, "-90", NULL);
  lat1 = scan_ctl(argv[1], argc, argv, "MAP_LAT1", -1, "90", NULL);
  dlat = scan_ctl(argv[1], argc, argv, "MAP_DLAT", -1, "-999", NULL);

  /* Loop over files... */
  for (i = 3; i < argc; i++) {

    /* Read meteorological data... */
    if (!read_met(&ctl, argv[i], met))
      continue;

    /* Set horizontal grid... */
    if (dlon <= 0)
      dlon = fabs(met->lon[1] - met->lon[0]);
    if (dlat <= 0)
      dlat = fabs(met->lat[1] - met->lat[0]);
    if (lon0 < -360 && lon1 > 360) {
      lon0 = gsl_stats_min(met->lon, 1, (size_t) met->nx);
      lon1 = gsl_stats_max(met->lon, 1, (size_t) met->nx);
    }
    nx = ny = 0;
    for (lon = lon0; lon <= lon1; lon += dlon) {
      lons[nx] = lon;
      if ((++nx) > NX)
	ERRMSG("Too many longitudes!");
    }
    if (lat0 < -90 && lat1 > 90) {
      lat0 = gsl_stats_min(met->lat, 1, (size_t) met->ny);
      lat1 = gsl_stats_max(met->lat, 1, (size_t) met->ny);
    }
    for (lat = lat0; lat <= lat1; lat += dlat) {
      lats[ny] = lat;
      if ((++ny) > NY)
	ERRMSG("Too many latitudes!");
    }

    /* Average... */
    for (ix = 0; ix < nx; ix++)
      for (iy = 0; iy < ny; iy++) {

	/* Interpolate meteo data... */
	intpol_met_space_3d(met, met->z, p0, lons[ix], lats[iy], &z, ci, cw,
			    1);
	intpol_met_space_3d(met, met->t, p0, lons[ix], lats[iy], &t, ci, cw,
			    0);
	intpol_met_space_3d(met, met->u, p0, lons[ix], lats[iy], &u, ci, cw,
			    0);
	intpol_met_space_3d(met, met->v, p0, lons[ix], lats[iy], &v, ci, cw,
			    0);
	intpol_met_space_3d(met, met->w, p0, lons[ix], lats[iy], &w, ci, cw,
			    0);
	intpol_met_space_3d(met, met->pv, p0, lons[ix], lats[iy], &pv, ci,
			    cw, 0);
	intpol_met_space_3d(met, met->h2o, p0, lons[ix], lats[iy], &h2o, ci,
			    cw, 0);
	intpol_met_space_3d(met, met->o3, p0, lons[ix], lats[iy], &o3, ci,
			    cw, 0);
	intpol_met_space_3d(met, met->lwc, p0, lons[ix], lats[iy], &lwc, ci,
			    cw, 0);
	intpol_met_space_3d(met, met->iwc, p0, lons[ix], lats[iy], &iwc, ci,
			    cw, 0);
	intpol_met_space_2d(met, met->ps, lons[ix], lats[iy], &ps, ci, cw, 0);
	intpol_met_space_2d(met, met->pt, lons[ix], lats[iy], &pt, ci, cw, 0);
	intpol_met_space_2d(met, met->pc, lons[ix], lats[iy], &pc, ci, cw, 0);
	intpol_met_space_2d(met, met->cl, lons[ix], lats[iy], &cl, ci, cw, 0);

	/* Interpolate tropopause data... */
	intpol_met_space_3d(met, met->z, pt, lons[ix], lats[iy], &zt, ci, cw,
			    1);
	intpol_met_space_3d(met, met->t, pt, lons[ix], lats[iy], &tt, ci, cw,
			    0);
	intpol_met_space_3d(met, met->h2o, pt, lons[ix], lats[iy], &h2ot, ci,
			    cw, 0);

	/* Averaging... */
	timem[ix][iy] += met->time;
	zm[ix][iy] += z;
	tm[ix][iy] += t;
	um[ix][iy] += u;
	vm[ix][iy] += v;
	wm[ix][iy] += w;
	pvm[ix][iy] += pv;
	h2om[ix][iy] += h2o;
	o3m[ix][iy] += o3;
	lwcm[ix][iy] += lwc;
	iwcm[ix][iy] += iwc;
	psm[ix][iy] += ps;
	ptm[ix][iy] += pt;
	pcm[ix][iy] += pc;
	clm[ix][iy] += cl;
	ztm[ix][iy] += zt;
	ttm[ix][iy] += tt;
	h2otm[ix][iy] += h2ot;
	np[ix][iy]++;
      }
  }

  /* Create output file... */
  printf("Write meteorological data file: %s\n", argv[2]);
  if (!(out = fopen(argv[2], "w")))
    ERRMSG("Cannot create file!");

  /* Write header... */
  fprintf(out,
	  "# $1 = time [s]\n"
	  "# $2 = altitude [km]\n"
	  "# $3 = longitude [deg]\n"
	  "# $4 = latitude [deg]\n"
	  "# $5 = pressure [hPa]\n"
	  "# $6 = temperature [K]\n"
	  "# $7 = zonal wind [m/s]\n"
	  "# $8 = meridional wind [m/s]\n"
	  "# $9 = vertical wind [hPa/s]\n"
	  "# $10 = H2O volume mixing ratio [ppv]\n");
  fprintf(out,
	  "# $11 = O3 volume mixing ratio [ppv]\n"
	  "# $12 = geopotential height [km]\n"
	  "# $13 = potential vorticity [PVU]\n"
	  "# $14 = surface pressure [hPa]\n"
	  "# $15 = tropopause pressure [hPa]\n"
	  "# $16 = tropopause geopotential height [km]\n"
	  "# $17 = tropopause temperature [K]\n"
	  "# $18 = tropopause water vapor [ppv]\n"
	  "# $19 = cloud liquid water content [kg/kg]\n"
	  "# $20 = cloud ice water content [kg/kg]\n");
  fprintf(out,
	  "# $21 = total column cloud water [kg/m^2]\n"
	  "# $22 = cloud top pressure [hPa]\n");

  /* Write data... */
  for (iy = 0; iy < ny; iy++) {
    fprintf(out, "\n");
    for (ix = 0; ix < nx; ix++)
      fprintf(out,
	      "%.2f %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g\n",
	      timem[ix][iy] / np[ix][iy], Z(p0), lons[ix], lats[iy], p0,
	      tm[ix][iy] / np[ix][iy], um[ix][iy] / np[ix][iy],
	      vm[ix][iy] / np[ix][iy], wm[ix][iy] / np[ix][iy],
	      h2om[ix][iy] / np[ix][iy], o3m[ix][iy] / np[ix][iy],
	      zm[ix][iy] / np[ix][iy], pvm[ix][iy] / np[ix][iy],
	      psm[ix][iy] / np[ix][iy], ptm[ix][iy] / np[ix][iy],
	      ztm[ix][iy] / np[ix][iy], ttm[ix][iy] / np[ix][iy],
	      h2otm[ix][iy] / np[ix][iy], lwcm[ix][iy] / np[ix][iy],
	      iwcm[ix][iy] / np[ix][iy], clm[ix][iy] / np[ix][iy],
	      pcm[ix][iy] / np[ix][iy]);
  }

  /* Close file... */
  fclose(out);

  /* Free... */
  free(met);

  return EXIT_SUCCESS;
}
