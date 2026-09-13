			/* Version Numbers - External */
static char version [] =
   "(C) Copyright 1985 TORCH Computers Ltd Version 0.10\nTriple X";

			/* Version Numbers Records */
/*
 * Version	- 	Reason for change.
 *
 *   0.10		Release for Beta Test to KRN 13/9/85
 *
 */

#include <math.h>

/*
 * Given three distinct points on the circumference of a circle work out the
 * origin.
 * Points (x1,y1), (x2,y2) and (x3,y3). The origin lies on a line which is
 * perpendicular to the chord between two of the points and cuts the chord in
 * the middle.
 * Therefore the gradient (m) is
 *
 *         m = (x2 - x1) / (y2 - y1) = (xi - x1) / (yi - y1)
 *
 * where (xi, yi) is the middle of the chord i.e.
 *
 *	xi = (x1 + x2) / 2 and yi = (y1 + y2) / 2
 *
 * The line perpendicular to the chord will have gradient -1 / m and since it
 * will pass through (xi, yi)
 *
 *	(x - xi) / (y - yi) = -((y2 - y1) / (x2 - x1))
 *
 * substituting for xi and yi and reshuffling eventually results in
 *
 *	(y2 - y1) * y + (x2 - x1) * x = (y2^2 - y1^2 + x2^2 - x1^2) / 2
 *
 * where a^2 indicates a to the power 2.
 * By using two pairs of points, two line equations can be created and the
 * point at which they meet can be evaluated.
 *
 *	( a b ) ( x ) = ( e )
 *	( c d ) ( y )   ( f )
 *
 * The above system gives
 *
 *	y = (f*a - c*e) / (a*d - c*b)	and	x = (f*b - d*e) / (a*d - c*b)
 *
 * If (a*d - c*b) = 0, then there is no solution. This could be because the
 * three points are colinear or two of the points are coincident. In this case
 * the program should use straight lines to join the points instead of arcs.
 */



float cnstfnc(p1, p2)
register int *p1, *p2; {
/*
 * work out the constant function.
 *	 (y2^2 - y1^2 + x2^2 - x1^2) / 2
 */
	float tmp1, tmp2, tmp3, tmp4;

	tmp1 = p1[0] * p1[0];
	tmp2 = p2[0] * p2[0];
	tmp3 = p1[1] * p1[1];
	tmp4 = p2[1] * p2[1];

	return( (tmp4 - tmp3 + tmp2 - tmp1) / 2.0);
}


solve(res, p1, p2, p3)
register int *res, *p1, *p2, *p3; {
/*
 * Each of the arguments points to two integers which are the x and y
 * coordinates of a point. If the origin of the arc is discovered, the function
 * will return true and the origin's coordinates will be passed back via res.
 * Otherwise the function will return false.
 * The x and y coordinates have maximum values of -32768 and 32767 which could
 * lead to overflow so floating point is used internally.
 */
	float arr[2][3];
	float denom;
	float tmp1, tmp2;

	arr[0][0] = p2[0] - p1[0];	/* a */
	arr[0][1] = p2[1] - p1[1];	/* b */
	arr[0][2] = cnstfnc(p1, p2);	/* e */

	arr[1][0] = p3[0] - p2[0];	/* c */
	arr[1][1] = p3[1] - p2[1];	/* d */
	arr[1][2] = cnstfnc(p2, p3);	/* f */

	denom = arr[0][0] * arr[1][1] - arr[1][0] * arr[0][1];
	/* denom = a*d - b*c */
	if (fabs(denom) < 0.5)
		return(0);

	tmp1 = ( arr[1][1] * arr[0][2] - arr[1][2] * arr[0][1] ) / denom;
	/* x = tmp1 = (d*e - f*b)/denom */
	tmp2 = ( arr[1][2] * arr[0][0] - arr[1][0] * arr[0][2] ) / denom;
	/* y = tmp2 = (f*a - c*e)/denom */
	if (fabs(tmp1) > 32767 || fabs(tmp2) > 32767)
		return(0);
	res[0] = tmp1;
	res[1] = tmp2;
	return(1);
}

cross(p1, p2, p3)
register int *p1, *p2, *p3; {
/*
 * p1, p2 and p3 are points. This routine returns the cross product of the
 * vectors p1p2 and p2p3. A returned negative value would indicate a clockwise
 * rotation. The most likely use for this routine is to decide in which
 * direction an arc will be drawn.
 */
	int arr[2][2];

	/* vector p1p2 */
	arr[0][0] = p2[0] - p1[0];
	arr[1][0] = p2[1] - p1[1];

	/* vector p1p3 */
	arr[0][1] = p3[0] - p1[0];
	arr[1][1] = p3[1] - p1[1];

	return(arr[0][0]*arr[1][1] - arr[1][0]*arr[0][1]);
}


