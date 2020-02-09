/*
This source code file is licensed under the GNU GPL Version 2.0 Licence by the following copyright holder:
Crown Copyright Commonwealth of Australia (Geoscience Australia) 2015.
The GNU GPL 2.0 licence is available at: http://www.gnu.org/licenses/gpl-2.0.html. If you require a paper copy of the GNU GPL 2.0 Licence, please write to Free Software Foundation, Inc. 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

Author: Ross C. Brodie, Geoscience Australia.
*/

#ifndef _general_constants_H
#define _general_constants_H

constexpr auto MUZERO = 12.56637061435917295384e-7; //Magnetic permeability of free space;
constexpr auto EZERO = 8.854e-12;  //Electrical permitivity of free space
constexpr auto UGC = 6.67384e-11; //Universal gravitational constant

constexpr double PI = 3.1415926535897931;
constexpr auto TWOPI = 6.2831853071795862;
constexpr auto THREEPI = 9.4247779607693793;
constexpr auto FOURPI = 12.5663706143591720;
constexpr auto PIONTWO = 1.5707963267948966;
constexpr auto PIONTHREE = 1.0471975511965976;
constexpr auto PIONFOUR = 0.7853981633974483;
constexpr auto ONEONPI = 0.3183098861837907;
constexpr auto ONEONTWOPI = 0.1591549430918954;
constexpr auto ONEONTHREEPI = 0.1061032953945969;
constexpr auto ONEONFOURPI = 0.0795774715459477;
constexpr auto THREEONFOURPI = 0.2387324146378430;

constexpr auto D2R = PI / 180.0; //Convert degrees to radians
constexpr auto R2D = 180.0 / PI; //Convert radians to degrees

constexpr auto NLOG10 = 2.3025850929940459; //Natrural log(10)

#endif
