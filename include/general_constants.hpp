/*
This source code file is licensed under the GNU GPL Version 2.0 Licence by the following copyright holder:
Crown Copyright Commonwealth of Australia (Geoscience Australia) 2015.
The GNU GPL 2.0 licence is available at: http://www.gnu.org/licenses/gpl-2.0.html. If you require a paper copy of the GNU GPL 2.0 Licence, please write to Free Software Foundation, Inc. 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

Author: Ross C. Brodie, Geoscience Australia.
*/

#pragma once

namespace CppUtils {

	template <typename T> constexpr T MUZERO = 12.56637061435917295384e-7; //Magnetic permeability of free space;
	template <typename T> constexpr T EZERO = 8.854e-12;  //Electrical permitivity of free space
	template <typename T> constexpr T UGC = 6.67384e-11; //Universal gravitational constant

	template <typename T> constexpr T PI = 3.1415926535897931;
	template <typename T> constexpr T TWOPI = 6.2831853071795862;
	template <typename T> constexpr T THREEPI = 9.4247779607693793;
	template <typename T> constexpr T FOURPI = 12.5663706143591720;
	template <typename T> constexpr T PIONTWO = 1.5707963267948966;
	template <typename T> constexpr T PIONTHREE = 1.0471975511965976;
	template <typename T> constexpr T PIONFOUR = 0.7853981633974483;
	template <typename T> constexpr T ONEONPI = 0.3183098861837907;
	template <typename T> constexpr T ONEONTWOPI = 0.1591549430918954;
	template <typename T> constexpr T ONEONTHREEPI = 0.1061032953945969;
	template <typename T> constexpr T ONEONFOURPI = 0.0795774715459477;
	template <typename T> constexpr T THREEONFOURPI = 0.2387324146378430;

	template <typename T> constexpr T D2R = 0.017453292519943295;// PI/180 Convert degrees to radians
	template <typename T> constexpr T R2D = 57.295779513082322865;// 180/PI Convert radians to degrees
	template <typename T> constexpr T NLOG10 = 2.3025850929940459; //Natrural log(10)
	template <typename T> constexpr T GOLDENRATIO = 0.38196601125010510;//Golden Ratio
};

