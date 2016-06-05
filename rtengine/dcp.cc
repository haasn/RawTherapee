/*
*  This file is part of RawTherapee.
*
*  Copyright (c) 2012 Oliver Duis <www.oliverduis.de>
*
*  RawTherapee is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation, either version 3 of the License, or
*  (at your option) any later version.
*
*  RawTherapee is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with RawTherapee.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <memory>
#include <cstring>

#include "dcp.h"
#include "iccmatrices.h"
#include "iccstore.h"
#include "rawimagesource.h"
#include "improcfun.h"
#include "rt_math.h"

using namespace std;
using namespace rtengine;
using namespace rtexif;

namespace
{

static const float adobe_camera_raw_default_curve[] = {
    0.00000f, 0.00078f, 0.00160f, 0.00242f,
    0.00314f, 0.00385f, 0.00460f, 0.00539f,
    0.00623f, 0.00712f, 0.00806f, 0.00906f,
    0.01012f, 0.01122f, 0.01238f, 0.01359f,
    0.01485f, 0.01616f, 0.01751f, 0.01890f,
    0.02033f, 0.02180f, 0.02331f, 0.02485f,
    0.02643f, 0.02804f, 0.02967f, 0.03134f,
    0.03303f, 0.03475f, 0.03648f, 0.03824f,
    0.04002f, 0.04181f, 0.04362f, 0.04545f,
    0.04730f, 0.04916f, 0.05103f, 0.05292f,
    0.05483f, 0.05675f, 0.05868f, 0.06063f,
    0.06259f, 0.06457f, 0.06655f, 0.06856f,
    0.07057f, 0.07259f, 0.07463f, 0.07668f,
    0.07874f, 0.08081f, 0.08290f, 0.08499f,
    0.08710f, 0.08921f, 0.09134f, 0.09348f,
    0.09563f, 0.09779f, 0.09996f, 0.10214f,
    0.10433f, 0.10652f, 0.10873f, 0.11095f,
    0.11318f, 0.11541f, 0.11766f, 0.11991f,
    0.12218f, 0.12445f, 0.12673f, 0.12902f,
    0.13132f, 0.13363f, 0.13595f, 0.13827f,
    0.14061f, 0.14295f, 0.14530f, 0.14765f,
    0.15002f, 0.15239f, 0.15477f, 0.15716f,
    0.15956f, 0.16197f, 0.16438f, 0.16680f,
    0.16923f, 0.17166f, 0.17410f, 0.17655f,
    0.17901f, 0.18148f, 0.18395f, 0.18643f,
    0.18891f, 0.19141f, 0.19391f, 0.19641f,
    0.19893f, 0.20145f, 0.20398f, 0.20651f,
    0.20905f, 0.21160f, 0.21416f, 0.21672f,
    0.21929f, 0.22185f, 0.22440f, 0.22696f,
    0.22950f, 0.23204f, 0.23458f, 0.23711f,
    0.23963f, 0.24215f, 0.24466f, 0.24717f,
    0.24967f, 0.25216f, 0.25465f, 0.25713f,
    0.25961f, 0.26208f, 0.26454f, 0.26700f,
    0.26945f, 0.27189f, 0.27433f, 0.27676f,
    0.27918f, 0.28160f, 0.28401f, 0.28641f,
    0.28881f, 0.29120f, 0.29358f, 0.29596f,
    0.29833f, 0.30069f, 0.30305f, 0.30540f,
    0.30774f, 0.31008f, 0.31241f, 0.31473f,
    0.31704f, 0.31935f, 0.32165f, 0.32395f,
    0.32623f, 0.32851f, 0.33079f, 0.33305f,
    0.33531f, 0.33756f, 0.33981f, 0.34205f,
    0.34428f, 0.34650f, 0.34872f, 0.35093f,
    0.35313f, 0.35532f, 0.35751f, 0.35969f,
    0.36187f, 0.36404f, 0.36620f, 0.36835f,
    0.37050f, 0.37264f, 0.37477f, 0.37689f,
    0.37901f, 0.38112f, 0.38323f, 0.38533f,
    0.38742f, 0.38950f, 0.39158f, 0.39365f,
    0.39571f, 0.39777f, 0.39982f, 0.40186f,
    0.40389f, 0.40592f, 0.40794f, 0.40996f,
    0.41197f, 0.41397f, 0.41596f, 0.41795f,
    0.41993f, 0.42191f, 0.42388f, 0.42584f,
    0.42779f, 0.42974f, 0.43168f, 0.43362f,
    0.43554f, 0.43747f, 0.43938f, 0.44129f,
    0.44319f, 0.44509f, 0.44698f, 0.44886f,
    0.45073f, 0.45260f, 0.45447f, 0.45632f,
    0.45817f, 0.46002f, 0.46186f, 0.46369f,
    0.46551f, 0.46733f, 0.46914f, 0.47095f,
    0.47275f, 0.47454f, 0.47633f, 0.47811f,
    0.47989f, 0.48166f, 0.48342f, 0.48518f,
    0.48693f, 0.48867f, 0.49041f, 0.49214f,
    0.49387f, 0.49559f, 0.49730f, 0.49901f,
    0.50072f, 0.50241f, 0.50410f, 0.50579f,
    0.50747f, 0.50914f, 0.51081f, 0.51247f,
    0.51413f, 0.51578f, 0.51742f, 0.51906f,
    0.52069f, 0.52232f, 0.52394f, 0.52556f,
    0.52717f, 0.52878f, 0.53038f, 0.53197f,
    0.53356f, 0.53514f, 0.53672f, 0.53829f,
    0.53986f, 0.54142f, 0.54297f, 0.54452f,
    0.54607f, 0.54761f, 0.54914f, 0.55067f,
    0.55220f, 0.55371f, 0.55523f, 0.55673f,
    0.55824f, 0.55973f, 0.56123f, 0.56271f,
    0.56420f, 0.56567f, 0.56715f, 0.56861f,
    0.57007f, 0.57153f, 0.57298f, 0.57443f,
    0.57587f, 0.57731f, 0.57874f, 0.58017f,
    0.58159f, 0.58301f, 0.58443f, 0.58583f,
    0.58724f, 0.58864f, 0.59003f, 0.59142f,
    0.59281f, 0.59419f, 0.59556f, 0.59694f,
    0.59830f, 0.59966f, 0.60102f, 0.60238f,
    0.60373f, 0.60507f, 0.60641f, 0.60775f,
    0.60908f, 0.61040f, 0.61173f, 0.61305f,
    0.61436f, 0.61567f, 0.61698f, 0.61828f,
    0.61957f, 0.62087f, 0.62216f, 0.62344f,
    0.62472f, 0.62600f, 0.62727f, 0.62854f,
    0.62980f, 0.63106f, 0.63232f, 0.63357f,
    0.63482f, 0.63606f, 0.63730f, 0.63854f,
    0.63977f, 0.64100f, 0.64222f, 0.64344f,
    0.64466f, 0.64587f, 0.64708f, 0.64829f,
    0.64949f, 0.65069f, 0.65188f, 0.65307f,
    0.65426f, 0.65544f, 0.65662f, 0.65779f,
    0.65897f, 0.66013f, 0.66130f, 0.66246f,
    0.66362f, 0.66477f, 0.66592f, 0.66707f,
    0.66821f, 0.66935f, 0.67048f, 0.67162f,
    0.67275f, 0.67387f, 0.67499f, 0.67611f,
    0.67723f, 0.67834f, 0.67945f, 0.68055f,
    0.68165f, 0.68275f, 0.68385f, 0.68494f,
    0.68603f, 0.68711f, 0.68819f, 0.68927f,
    0.69035f, 0.69142f, 0.69249f, 0.69355f,
    0.69461f, 0.69567f, 0.69673f, 0.69778f,
    0.69883f, 0.69988f, 0.70092f, 0.70196f,
    0.70300f, 0.70403f, 0.70506f, 0.70609f,
    0.70711f, 0.70813f, 0.70915f, 0.71017f,
    0.71118f, 0.71219f, 0.71319f, 0.71420f,
    0.71520f, 0.71620f, 0.71719f, 0.71818f,
    0.71917f, 0.72016f, 0.72114f, 0.72212f,
    0.72309f, 0.72407f, 0.72504f, 0.72601f,
    0.72697f, 0.72794f, 0.72890f, 0.72985f,
    0.73081f, 0.73176f, 0.73271f, 0.73365f,
    0.73460f, 0.73554f, 0.73647f, 0.73741f,
    0.73834f, 0.73927f, 0.74020f, 0.74112f,
    0.74204f, 0.74296f, 0.74388f, 0.74479f,
    0.74570f, 0.74661f, 0.74751f, 0.74842f,
    0.74932f, 0.75021f, 0.75111f, 0.75200f,
    0.75289f, 0.75378f, 0.75466f, 0.75555f,
    0.75643f, 0.75730f, 0.75818f, 0.75905f,
    0.75992f, 0.76079f, 0.76165f, 0.76251f,
    0.76337f, 0.76423f, 0.76508f, 0.76594f,
    0.76679f, 0.76763f, 0.76848f, 0.76932f,
    0.77016f, 0.77100f, 0.77183f, 0.77267f,
    0.77350f, 0.77432f, 0.77515f, 0.77597f,
    0.77680f, 0.77761f, 0.77843f, 0.77924f,
    0.78006f, 0.78087f, 0.78167f, 0.78248f,
    0.78328f, 0.78408f, 0.78488f, 0.78568f,
    0.78647f, 0.78726f, 0.78805f, 0.78884f,
    0.78962f, 0.79040f, 0.79118f, 0.79196f,
    0.79274f, 0.79351f, 0.79428f, 0.79505f,
    0.79582f, 0.79658f, 0.79735f, 0.79811f,
    0.79887f, 0.79962f, 0.80038f, 0.80113f,
    0.80188f, 0.80263f, 0.80337f, 0.80412f,
    0.80486f, 0.80560f, 0.80634f, 0.80707f,
    0.80780f, 0.80854f, 0.80926f, 0.80999f,
    0.81072f, 0.81144f, 0.81216f, 0.81288f,
    0.81360f, 0.81431f, 0.81503f, 0.81574f,
    0.81645f, 0.81715f, 0.81786f, 0.81856f,
    0.81926f, 0.81996f, 0.82066f, 0.82135f,
    0.82205f, 0.82274f, 0.82343f, 0.82412f,
    0.82480f, 0.82549f, 0.82617f, 0.82685f,
    0.82753f, 0.82820f, 0.82888f, 0.82955f,
    0.83022f, 0.83089f, 0.83155f, 0.83222f,
    0.83288f, 0.83354f, 0.83420f, 0.83486f,
    0.83552f, 0.83617f, 0.83682f, 0.83747f,
    0.83812f, 0.83877f, 0.83941f, 0.84005f,
    0.84069f, 0.84133f, 0.84197f, 0.84261f,
    0.84324f, 0.84387f, 0.84450f, 0.84513f,
    0.84576f, 0.84639f, 0.84701f, 0.84763f,
    0.84825f, 0.84887f, 0.84949f, 0.85010f,
    0.85071f, 0.85132f, 0.85193f, 0.85254f,
    0.85315f, 0.85375f, 0.85436f, 0.85496f,
    0.85556f, 0.85615f, 0.85675f, 0.85735f,
    0.85794f, 0.85853f, 0.85912f, 0.85971f,
    0.86029f, 0.86088f, 0.86146f, 0.86204f,
    0.86262f, 0.86320f, 0.86378f, 0.86435f,
    0.86493f, 0.86550f, 0.86607f, 0.86664f,
    0.86720f, 0.86777f, 0.86833f, 0.86889f,
    0.86945f, 0.87001f, 0.87057f, 0.87113f,
    0.87168f, 0.87223f, 0.87278f, 0.87333f,
    0.87388f, 0.87443f, 0.87497f, 0.87552f,
    0.87606f, 0.87660f, 0.87714f, 0.87768f,
    0.87821f, 0.87875f, 0.87928f, 0.87981f,
    0.88034f, 0.88087f, 0.88140f, 0.88192f,
    0.88244f, 0.88297f, 0.88349f, 0.88401f,
    0.88453f, 0.88504f, 0.88556f, 0.88607f,
    0.88658f, 0.88709f, 0.88760f, 0.88811f,
    0.88862f, 0.88912f, 0.88963f, 0.89013f,
    0.89063f, 0.89113f, 0.89163f, 0.89212f,
    0.89262f, 0.89311f, 0.89360f, 0.89409f,
    0.89458f, 0.89507f, 0.89556f, 0.89604f,
    0.89653f, 0.89701f, 0.89749f, 0.89797f,
    0.89845f, 0.89892f, 0.89940f, 0.89987f,
    0.90035f, 0.90082f, 0.90129f, 0.90176f,
    0.90222f, 0.90269f, 0.90316f, 0.90362f,
    0.90408f, 0.90454f, 0.90500f, 0.90546f,
    0.90592f, 0.90637f, 0.90683f, 0.90728f,
    0.90773f, 0.90818f, 0.90863f, 0.90908f,
    0.90952f, 0.90997f, 0.91041f, 0.91085f,
    0.91130f, 0.91173f, 0.91217f, 0.91261f,
    0.91305f, 0.91348f, 0.91392f, 0.91435f,
    0.91478f, 0.91521f, 0.91564f, 0.91606f,
    0.91649f, 0.91691f, 0.91734f, 0.91776f,
    0.91818f, 0.91860f, 0.91902f, 0.91944f,
    0.91985f, 0.92027f, 0.92068f, 0.92109f,
    0.92150f, 0.92191f, 0.92232f, 0.92273f,
    0.92314f, 0.92354f, 0.92395f, 0.92435f,
    0.92475f, 0.92515f, 0.92555f, 0.92595f,
    0.92634f, 0.92674f, 0.92713f, 0.92753f,
    0.92792f, 0.92831f, 0.92870f, 0.92909f,
    0.92947f, 0.92986f, 0.93025f, 0.93063f,
    0.93101f, 0.93139f, 0.93177f, 0.93215f,
    0.93253f, 0.93291f, 0.93328f, 0.93366f,
    0.93403f, 0.93440f, 0.93478f, 0.93515f,
    0.93551f, 0.93588f, 0.93625f, 0.93661f,
    0.93698f, 0.93734f, 0.93770f, 0.93807f,
    0.93843f, 0.93878f, 0.93914f, 0.93950f,
    0.93986f, 0.94021f, 0.94056f, 0.94092f,
    0.94127f, 0.94162f, 0.94197f, 0.94231f,
    0.94266f, 0.94301f, 0.94335f, 0.94369f,
    0.94404f, 0.94438f, 0.94472f, 0.94506f,
    0.94540f, 0.94573f, 0.94607f, 0.94641f,
    0.94674f, 0.94707f, 0.94740f, 0.94774f,
    0.94807f, 0.94839f, 0.94872f, 0.94905f,
    0.94937f, 0.94970f, 0.95002f, 0.95035f,
    0.95067f, 0.95099f, 0.95131f, 0.95163f,
    0.95194f, 0.95226f, 0.95257f, 0.95289f,
    0.95320f, 0.95351f, 0.95383f, 0.95414f,
    0.95445f, 0.95475f, 0.95506f, 0.95537f,
    0.95567f, 0.95598f, 0.95628f, 0.95658f,
    0.95688f, 0.95718f, 0.95748f, 0.95778f,
    0.95808f, 0.95838f, 0.95867f, 0.95897f,
    0.95926f, 0.95955f, 0.95984f, 0.96013f,
    0.96042f, 0.96071f, 0.96100f, 0.96129f,
    0.96157f, 0.96186f, 0.96214f, 0.96242f,
    0.96271f, 0.96299f, 0.96327f, 0.96355f,
    0.96382f, 0.96410f, 0.96438f, 0.96465f,
    0.96493f, 0.96520f, 0.96547f, 0.96574f,
    0.96602f, 0.96629f, 0.96655f, 0.96682f,
    0.96709f, 0.96735f, 0.96762f, 0.96788f,
    0.96815f, 0.96841f, 0.96867f, 0.96893f,
    0.96919f, 0.96945f, 0.96971f, 0.96996f,
    0.97022f, 0.97047f, 0.97073f, 0.97098f,
    0.97123f, 0.97149f, 0.97174f, 0.97199f,
    0.97223f, 0.97248f, 0.97273f, 0.97297f,
    0.97322f, 0.97346f, 0.97371f, 0.97395f,
    0.97419f, 0.97443f, 0.97467f, 0.97491f,
    0.97515f, 0.97539f, 0.97562f, 0.97586f,
    0.97609f, 0.97633f, 0.97656f, 0.97679f,
    0.97702f, 0.97725f, 0.97748f, 0.97771f,
    0.97794f, 0.97817f, 0.97839f, 0.97862f,
    0.97884f, 0.97907f, 0.97929f, 0.97951f,
    0.97973f, 0.97995f, 0.98017f, 0.98039f,
    0.98061f, 0.98082f, 0.98104f, 0.98125f,
    0.98147f, 0.98168f, 0.98189f, 0.98211f,
    0.98232f, 0.98253f, 0.98274f, 0.98295f,
    0.98315f, 0.98336f, 0.98357f, 0.98377f,
    0.98398f, 0.98418f, 0.98438f, 0.98458f,
    0.98478f, 0.98498f, 0.98518f, 0.98538f,
    0.98558f, 0.98578f, 0.98597f, 0.98617f,
    0.98636f, 0.98656f, 0.98675f, 0.98694f,
    0.98714f, 0.98733f, 0.98752f, 0.98771f,
    0.98789f, 0.98808f, 0.98827f, 0.98845f,
    0.98864f, 0.98882f, 0.98901f, 0.98919f,
    0.98937f, 0.98955f, 0.98973f, 0.98991f,
    0.99009f, 0.99027f, 0.99045f, 0.99063f,
    0.99080f, 0.99098f, 0.99115f, 0.99133f,
    0.99150f, 0.99167f, 0.99184f, 0.99201f,
    0.99218f, 0.99235f, 0.99252f, 0.99269f,
    0.99285f, 0.99302f, 0.99319f, 0.99335f,
    0.99351f, 0.99368f, 0.99384f, 0.99400f,
    0.99416f, 0.99432f, 0.99448f, 0.99464f,
    0.99480f, 0.99495f, 0.99511f, 0.99527f,
    0.99542f, 0.99558f, 0.99573f, 0.99588f,
    0.99603f, 0.99619f, 0.99634f, 0.99649f,
    0.99664f, 0.99678f, 0.99693f, 0.99708f,
    0.99722f, 0.99737f, 0.99751f, 0.99766f,
    0.99780f, 0.99794f, 0.99809f, 0.99823f,
    0.99837f, 0.99851f, 0.99865f, 0.99879f,
    0.99892f, 0.99906f, 0.99920f, 0.99933f,
    0.99947f, 0.99960f, 0.99974f, 0.99987f,
    1.00000f
};

struct ruvt {
    double r;
    double u;
    double v;
    double t;
};

static const ruvt kTempTable [] = {
    {   0, 0.18006, 0.26352, -0.24341 },
    {  10, 0.18066, 0.26589, -0.25479 },
    {  20, 0.18133, 0.26846, -0.26876 },
    {  30, 0.18208, 0.27119, -0.28539 },
    {  40, 0.18293, 0.27407, -0.30470 },
    {  50, 0.18388, 0.27709, -0.32675 },
    {  60, 0.18494, 0.28021, -0.35156 },
    {  70, 0.18611, 0.28342, -0.37915 },
    {  80, 0.18740, 0.28668, -0.40955 },
    {  90, 0.18880, 0.28997, -0.44278 },
    { 100, 0.19032, 0.29326, -0.47888 },
    { 125, 0.19462, 0.30141, -0.58204 },
    { 150, 0.19962, 0.30921, -0.70471 },
    { 175, 0.20525, 0.31647, -0.84901 },
    { 200, 0.21142, 0.32312, -1.0182 },
    { 225, 0.21807, 0.32909, -1.2168 },
    { 250, 0.22511, 0.33439, -1.4512 },
    { 275, 0.23247, 0.33904, -1.7298 },
    { 300, 0.24010, 0.34308, -2.0637 },
    { 325, 0.24702, 0.34655, -2.4681 },
    { 350, 0.25591, 0.34951, -2.9641 },
    { 375, 0.26400, 0.35200, -3.5814 },
    { 400, 0.27218, 0.35407, -4.3633 },
    { 425, 0.28039, 0.35577, -5.3762 },
    { 450, 0.28863, 0.35714, -6.7262 },
    { 475, 0.29685, 0.35823, -8.5955 },
    { 500, 0.30505, 0.35907, -11.324 },
    { 525, 0.31320, 0.35968, -15.628 },
    { 550, 0.32129, 0.36011, -23.325 },
    { 575, 0.32931, 0.36038, -40.770 },
    { 600, 0.33724, 0.36051, -116.45 }
};

// This sRGB gamma is taken from DNG reference code, with the added linear extension past 1.0, as we run clipless here
float sRGBGammaForward(float x)
{
    return
        x <= 0.0031308f
            ? x * 12.92f
            : x > 1.0f
                ? 1.0f + (x - 1.0f) * (1.055f * (1.0f / 2.4f)) // Linear extension
                : 1.055f * pow(x, 1.0f / 2.4f) - 0.055f;
}

float sRGBGammaInverse(float y)
{
    return
        y <= 0.0031308f * 12.92f
            ? y * (1.0f / 12.92f)
            : y > 1.0f
                ? 1.0f + (y - 1.0f) / (1.055f * (1.0f / 2.4f))
                : pow ((y + 0.055f) * (1.0f / 1.055f), 2.4f);
}

void invert3x3(const double (*A)[3], double (*B)[3])
{
    const double& a00 = A[0][0];
    const double& a01 = A[0][1];
    const double& a02 = A[0][2];
    const double& a10 = A[1][0];
    const double& a11 = A[1][1];
    const double& a12 = A[1][2];
    const double& a20 = A[2][0];
    const double& a21 = A[2][1];
    const double& a22 = A[2][2];

    double temp[3][3];

    temp[0][0] = a11 * a22 - a21 * a12;
    temp[0][1] = a21 * a02 - a01 * a22;
    temp[0][2] = a01 * a12 - a11 * a02;
    temp[1][0] = a20 * a12 - a10 * a22;
    temp[1][1] = a00 * a22 - a20 * a02;
    temp[1][2] = a10 * a02 - a00 * a12;
    temp[2][0] = a10 * a21 - a20 * a11;
    temp[2][1] = a20 * a01 - a00 * a21;
    temp[2][2] = a00 * a11 - a10 * a01;

    const double det = a00 * temp[0][0] + a01 * temp[1][0] + a02 * temp[2][0];

    if (fabs(det) < 1.0E-10) {
        abort(); // Can't be inverted, we shouldn't be dealing with such matrices
    }

    for (int j = 0; j < 3; ++j) {
        for (int k = 0; k < 3; ++k) {
            B[j][k] = temp[j][k] / det;
        }
    }
}

void multiply3x3(const double (*A)[3], const double (*B)[3], double (*C)[3])
{
    // Use temp to support having output same as input
    double M[3][3];

    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            M[i][j] = 0;

            for (int k = 0; k < 3; ++k) {
                M[i][j] += A[i][k] * B[k][j];
            }
        }
    }

    memcpy(C, M, 3 * 3 * sizeof(double));
}

void multiply3x3_v3(const double (*A)[3], const double B[3], double C[3])
{
    // Use temp to support having output same as input
    double M[3] = {};

    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            M[i] += A[i][j] * B[j];
        }
    }

    memcpy(C, M, 3 * sizeof(double));
}

void mix3x3(const double (*A)[3], double mulA, const double (*B)[3], double mulB, double (*C)[3])
{
    double M[3][3];

    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            M[i][j] = A[i][j] * mulA + B[i][j] * mulB;
        }
    }

    memcpy(C, M, 3 * 3 * sizeof(double));
}

void mapWhiteMatrix(const double white1[3], const double white2[3], double (*B)[3])
{
    // Code adapted from dng_color_spec::MapWhiteMatrix

    // Use the linearized Bradford adaptation matrix
    const double Mb[3][3] = {
        { 0.8951,  0.2664, -0.1614 },
        { -0.7502, 1.7135,  0.0367 },
        { 0.0389, -0.0685,  1.0296 }
    };

    double w1[3];
    multiply3x3_v3(Mb, white1, w1);
    double w2[3];
    multiply3x3_v3(Mb, white2, w2);

    // Negative white coordinates are kind of meaningless.
    w1[0] = std::max(w1[0], 0.0);
    w1[1] = std::max(w1[1], 0.0);
    w1[2] = std::max(w1[2], 0.0);
    w2[0] = std::max(w2[0], 0.0);
    w2[1] = std::max(w2[1], 0.0);
    w2[2] = std::max(w2[2], 0.0);

    // Limit scaling to something reasonable.
    double A[3][3] = {};
    A[0][0] = std::max(0.1, std::min(w1[0] > 0.0 ? w2[0] / w1[0] : 10.0, 10.0));
    A[1][1] = std::max(0.1, std::min(w1[1] > 0.0 ? w2[1] / w1[1] : 10.0, 10.0));
    A[2][2] = std::max(0.1, std::min(w1[2] > 0.0 ? w2[2] / w1[2] : 10.0, 10.0));

    double temp[3][3];
    invert3x3(Mb, temp);
    multiply3x3(temp, A, temp);
    multiply3x3(temp, Mb, B);
}

void XYZtoXY(const double XYZ[3], double XY[2])
{
    const double& X = XYZ[0];
    const double& Y = XYZ[1];
    const double& Z = XYZ[2];

    const double total = X + Y + Z;

    if (total > 0.0) {
        XY[0] = X / total;
        XY[1] = Y / total;
    } else {
        XY[0] = 0.3457;
        XY[1] = 0.3585;
    }
}

void XYtoXYZ(const double XY[2], double XYZ[3])
{
    double temp[2] = { XY[0], XY[1] };

    // Restrict xy coord to someplace inside the range of real xy coordinates.
    // This prevents math from doing strange things when users specify
    // extreme temperature/tint coordinates.
    temp[0] = std::max(0.000001, std::min(temp[0], 0.999999));
    temp[1] = std::max(0.000001, std::min(temp[1], 0.999999));

    if (temp[0] + temp[1] > 0.999999) {
        double scale = 0.999999 / (temp[0] + temp[1]);
        temp[0] *= scale;
        temp[1] *= scale;
    }

    XYZ[0] = temp[0] / temp[1];
    XYZ[1] = 1.0;
    XYZ[2] = (1.0 - temp[0] - temp[1]) / temp[1];
}

double calibrationIlluminantToTemperature(int light)
{
    enum class LightSource {
        UNKNOWN = 0,
        DAYLIGHT = 1,
        FLUORESCENT = 2,
        TUNGSTEN = 3,
        FLASH = 4,
        FINE_WEATHER = 9,
        CLOUDY_WEATHER = 10,
        SHADE = 11,
        DAYLIGHT_FLUORESCENT = 12, // D  5700 - 7100K
        DAYWHITE_FLUORESCENT = 13, // N  4600 - 5500K
        COOL_WHITE_FLUORESCENT = 14, // W  3800 - 4500K
        WHITE_FLUORESCENT = 15, // WW 3250 - 3800K
        WARM_WHITE_FLUORESCENT = 16, // L  2600 - 3250K
        STANDARD_LIGHT_A = 17,
        STANDARD_LIGHT_B = 18,
        STANDARD_LIGHT_C = 19,
        D55 = 20,
        D65 = 21,
        D75 = 22,
        D50 = 23,
        ISO_STUDIO_TUNGSTEN = 24,
        OTHER = 255
    };

    // These temperatures are those found in DNG SDK reference code
    switch (LightSource(light)) {
        case LightSource::STANDARD_LIGHT_A:
        case LightSource::TUNGSTEN: {
            return 2850.0;
        }

        case LightSource::ISO_STUDIO_TUNGSTEN: {
            return 3200.0;
        }

        case LightSource::D50: {
            return 5000.0;
        }

        case LightSource::D55:
        case LightSource::DAYLIGHT:
        case LightSource::FINE_WEATHER:
        case LightSource::FLASH:
        case LightSource::STANDARD_LIGHT_B: {
            return 5500.0;
        }

        case LightSource::D65:
        case LightSource::STANDARD_LIGHT_C:
        case LightSource::CLOUDY_WEATHER: {
            return 6500.0;
        }

        case LightSource::D75:
        case LightSource::SHADE: {
            return 7500.0;
        }

        case LightSource::DAYLIGHT_FLUORESCENT: {
            return (5700.0 + 7100.0) * 0.5;
        }

        case LightSource::DAYWHITE_FLUORESCENT: {
            return (4600.0 + 5500.0) * 0.5;
        }

        case LightSource::COOL_WHITE_FLUORESCENT:
        case LightSource::FLUORESCENT: {
            return (3800.0 + 4500.0) * 0.5;
        }

        case LightSource::WHITE_FLUORESCENT: {
            return (3250.0 + 3800.0) * 0.5;
        }

        case LightSource::WARM_WHITE_FLUORESCENT: {
            return (2600.0 + 3250.0) * 0.5;
        }

        default: {
            return 0.0;
        }
    }
}

}

DCPProfile::DCPProfile(const Glib::ustring& filename) :
    has_color_matrix_1(false),
    has_color_matrix_2(false),
    has_forward_matrix_1(false),
    has_forward_matrix_2(false),
    has_tone_curve(false),
    has_baseline_exposure_offset(false),
    will_interpolate(false),
    baseline_exposure_offset(0.0),
    deltas_1(nullptr),
    deltas_2(nullptr),
    look_table(nullptr)
{
    constexpr int tiff_float_size = 4;

    enum class TagKey : int {
        COLOR_MATRIX_1 = 50721,
        COLOR_MATRIX_2 = 50722,
        PROFILE_HUE_SAT_MAP_DIMS = 50937,
        PROFILE_HUE_SAT_MAP_DATA_1 = 50938,
        PROFILE_HUE_SAT_MAP_DATA_2 = 50939,
        PROFILE_TONE_CURVE = 50940,
        PROFILE_TONE_COPYRIGHT = 50942,
        CALIBRATION_ILLUMINANT_1 = 50778,
        CALIBRATION_ILLUMINANT_2 = 50779,
        FORWARD_MATRIX_1 = 50964,
        FORWARD_MATRIX_2 = 50965,
        PROFILE_LOOK_TABLE_DIMS = 50981, // ProfileLookup is the low quality variant
        PROFILE_LOOK_TABLE_DATA = 50982,
        PROFILE_HUE_SAT_MAP_ENCODING = 51107,
        PROFILE_LOOK_TABLE_ENCODING = 51108,
        BASELINE_EXPOSURE_OFFSET = 51109
    };

    FILE* const file = g_fopen(filename.c_str(), "rb");

    std::unique_ptr<TagDirectory> tagDir(ExifManager::parseTIFF(file, false));

    Tag* tag = tagDir->getTag(toUnderlying(TagKey::CALIBRATION_ILLUMINANT_1));
    light_source_1 =
        tag
            ? tag->toInt(0, rtexif::SHORT)
            : -1;
    tag = tagDir->getTag(toUnderlying(TagKey::CALIBRATION_ILLUMINANT_2));
    light_source_2 =
        tag
            ? tag->toInt(0, rtexif::SHORT)
            : -1;
    temperature_1 = calibrationIlluminantToTemperature(light_source_1);
    temperature_2 = calibrationIlluminantToTemperature(light_source_2);

    const bool has_second_hue_sat = tagDir->getTag(toUnderlying(TagKey::PROFILE_HUE_SAT_MAP_DATA_2)); // Some profiles have two matrices, but just one huesat

    // Fetch Forward Matrices, if any
    tag = tagDir->getTag(toUnderlying(TagKey::FORWARD_MATRIX_1));

    if (tag) {
        has_forward_matrix_1 = true;

        for (int row = 0; row < 3; ++row) {
            for (int col = 0; col < 3; ++col) {
                forward_matrix_1[row][col] = tag->toDouble((col + row * 3) * 8);
            }
        }
    }

    tag = tagDir->getTag(toUnderlying(TagKey::FORWARD_MATRIX_2));

    if (tag) {
        has_forward_matrix_2 = true;

        for (int row = 0; row < 3; ++row) {
            for (int col = 0; col < 3; ++col) {
                forward_matrix_2[row][col] = tag->toDouble((col + row * 3) * 8);
            }
        }
    }

    // Color Matrix (one is always there)
    tag = tagDir->getTag(toUnderlying(TagKey::COLOR_MATRIX_1));

    if (!tag) {
        // FIXME: better error handling
        fprintf(stderr, "Bad DCP, no ColorMatrix1\n");
        abort();
    }

    has_color_matrix_1 = true;

    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 3; ++col) {
            color_matrix_1[row][col] = tag->toDouble((col + row * 3) * 8);
        }
    }

    tag = tagDir->getTag(toUnderlying(TagKey::PROFILE_LOOK_TABLE_DIMS));

    if (tag) {
        look_info.hue_divisions = tag->toInt(0);
        look_info.sat_divisions = tag->toInt(4);
        look_info.val_divisions = tag->toInt(8);

        tag = tagDir->getTag(toUnderlying(TagKey::PROFILE_LOOK_TABLE_ENCODING));
        look_info.srgb_gamma = tag && tag->toInt(0);

        tag = tagDir->getTag(toUnderlying(TagKey::PROFILE_LOOK_TABLE_DATA));
        look_info.array_count = tag->getCount() / 3;

        look_table = new HSBModify[look_info.array_count];

        for (int i = 0; i < look_info.array_count; i++) {
            look_table[i].hue_shift = tag->toDouble((i * 3) * tiff_float_size);
            look_table[i].sat_scale = tag->toDouble((i * 3 + 1) * tiff_float_size);
            look_table[i].val_scale = tag->toDouble((i * 3 + 2) * tiff_float_size);
        }

        // Precalculated constants for table application
        look_info.pc.h_scale =
            look_info.hue_divisions < 2
                ? 0.0f
                : static_cast<float>(look_info.hue_divisions) / 6.0f;
        look_info.pc.s_scale = look_info.sat_divisions - 1;
        look_info.pc.v_scale = look_info.val_divisions - 1;
        look_info.pc.maxHueIndex0 = look_info.hue_divisions - 1;
        look_info.pc.maxSatIndex0 = look_info.sat_divisions - 2;
        look_info.pc.maxValIndex0 = look_info.val_divisions - 2;
        look_info.pc.hueStep = look_info.sat_divisions;
        look_info.pc.valStep = look_info.hue_divisions * look_info.pc.hueStep;
    }

    tag = tagDir->getTag(toUnderlying(TagKey::PROFILE_HUE_SAT_MAP_DIMS));

    if (tag) {
        delta_info.hue_divisions = tag->toInt(0);
        delta_info.sat_divisions = tag->toInt(4);
        delta_info.val_divisions = tag->toInt(8);

        tag = tagDir->getTag(toUnderlying(TagKey::PROFILE_HUE_SAT_MAP_ENCODING));
        delta_info.srgb_gamma = tag && tag->toInt(0);

        tag = tagDir->getTag(toUnderlying(TagKey::PROFILE_HUE_SAT_MAP_DATA_1));
        delta_info.array_count = tag->getCount() / 3;

        deltas_1 = new HSBModify[delta_info.array_count];

        for (int i = 0; i < delta_info.array_count; ++i) {
            deltas_1[i].hue_shift = tag->toDouble((i * 3) * tiff_float_size);
            deltas_1[i].sat_scale = tag->toDouble((i * 3 + 1) * tiff_float_size);
            deltas_1[i].val_scale = tag->toDouble((i * 3 + 2) * tiff_float_size);
        }

        delta_info.pc.h_scale =
            delta_info.hue_divisions < 2
                ? 0.0f
                : static_cast<float>(delta_info.hue_divisions) / 6.0f;
        delta_info.pc.s_scale = delta_info.sat_divisions - 1;
        delta_info.pc.v_scale = delta_info.val_divisions - 1;
        delta_info.pc.maxHueIndex0 = delta_info.hue_divisions - 1;
        delta_info.pc.maxSatIndex0 = delta_info.sat_divisions - 2;
        delta_info.pc.maxValIndex0 = delta_info.val_divisions - 2;
        delta_info.pc.hueStep = delta_info.sat_divisions;
        delta_info.pc.valStep = delta_info.hue_divisions * delta_info.pc.hueStep;
    }

    if (light_source_2 != -1) {
        // Second matrix
        has_color_matrix_2 = true;

        tag = tagDir->getTag(toUnderlying(TagKey::COLOR_MATRIX_2));

        for (int row = 0; row < 3; ++row) {
            for (int col = 0; col < 3; ++col) {
                color_matrix_2[row][col] =
                    tag
                        ? tag->toDouble((col + row * 3) * 8)
                        : color_matrix_1[row][col];
            }
        }

        // Second huesatmap
        if (has_second_hue_sat) {
            deltas_2 = new HSBModify[delta_info.array_count];

            // Saturation maps. Need to be unwinded.
            tag = tagDir->getTag(toUnderlying(TagKey::PROFILE_HUE_SAT_MAP_DATA_2));

            for (int i = 0; i < delta_info.array_count; ++i) {
                deltas_2[i].hue_shift = tag->toDouble((i * 3) * tiff_float_size);
                deltas_2[i].sat_scale = tag->toDouble((i * 3 + 1) * tiff_float_size);
                deltas_2[i].val_scale = tag->toDouble((i * 3 + 2) * tiff_float_size);
            }
        }
    }

    tag = tagDir->getTag(toUnderlying(TagKey::BASELINE_EXPOSURE_OFFSET));

    if (tag) {
        has_baseline_exposure_offset = true;
        baseline_exposure_offset = tag->toDouble();
    }

    // Read tone curve points, if any, but disable to RTs own profiles
    tag = tagDir->getTag(toUnderlying(TagKey::PROFILE_TONE_CURVE));

    if (tag) {
        std::vector<double> curve_points = {
            static_cast<double>(DCT_Spline) // The first value is the curve type
        };

        // Push back each X/Y coordinates in a loop
        bool curve_is_linear = true;

        for (int i = 0; i < tag->getCount(); i += 2) {
            const double x = tag->toDouble((i + 0) * tiff_float_size);
            const double y = tag->toDouble((i + 1) * tiff_float_size);

            if (x != y) {
                curve_is_linear = false;
            }

            curve_points.push_back(x);
            curve_points.push_back(y);
        }

        if (!curve_is_linear) {
            // Create the curve
            has_tone_curve = true;
            tone_curve.Set(DiagonalCurve(curve_points, CURVES_MIN_POLY_POINTS));
        }
    } else {
        tag = tagDir->getTag(toUnderlying(TagKey::PROFILE_TONE_COPYRIGHT));

        if (tag && tag->valueToString().find("Adobe Systems") != std::string::npos) {
            // An Adobe profile without tone curve is expected to have the Adobe Default Curve, we add that
            std::vector<double> curve_points = {
                static_cast<double>(DCT_Spline)
            };

            constexpr size_t tc_len = sizeof(adobe_camera_raw_default_curve) / sizeof(adobe_camera_raw_default_curve[0]);

            for (size_t i = 0; i < tc_len; ++i) {
                const double x = static_cast<double>(i) / (tc_len - 1);
                const double y = adobe_camera_raw_default_curve[i];
                curve_points.push_back(x);
                curve_points.push_back(y);
            }

            has_tone_curve = true;
            tone_curve.Set(DiagonalCurve(curve_points, CURVES_MIN_POLY_POINTS));
        }
    }

    will_interpolate = false;

    if (has_forward_matrix_1) {
        if (has_forward_matrix_2) {
            if (memcmp(forward_matrix_1, forward_matrix_2, sizeof(forward_matrix_1)) != 0) {
                // Common that forward matrices are the same!
                will_interpolate = true;
            }

            if (deltas_1 && deltas_2) {
                // We assume tables are different
                will_interpolate = true;
            }
        }
    }

    if (has_color_matrix_1 && has_color_matrix_2) {
        if (memcmp(color_matrix_1, color_matrix_2, sizeof(color_matrix_1)) != 0) {
            will_interpolate = true;
        }

        if (deltas_1 && deltas_2) {
            will_interpolate = true;
        }
    }

    if (file) {
        fclose(file);
    }
}

DCPProfile::~DCPProfile()
{
    delete[] deltas_1;
    delete[] deltas_2;
    delete[] look_table;
}

bool DCPProfile::getHasToneCurve() const
{
    return has_tone_curve;
}

bool DCPProfile::getHasLookTable() const
{
    return look_table;
}

bool DCPProfile::getHasHueSatMap() const
{
    return deltas_1;
}

bool DCPProfile::getHasBaselineExposureOffset() const
{
    return has_baseline_exposure_offset;
}

void DCPProfile::getIlluminants(int &i1, double &temp1, int &i2, double &temp2, bool &willInterpolate_) const
{
    i1 = light_source_1;
    i2 = light_source_2;
    temp1 = temperature_1, temp2 = temperature_2;
    willInterpolate_ = will_interpolate;
};

void DCPProfile::Apply(Imagefloat *pImg, int preferredIlluminant, const Glib::ustring &workingSpace, const ColorTemp &wb, double pre_mul[3], double camWbMatrix[3][3], bool useToneCurve, bool applyHueSatMap, bool applyLookTable) const
{

    TMatrix mWork = iccStore->workingSpaceInverseMatrix (workingSpace);

    double mXYZCAM[3][3]; // Camera RGB to XYZ D50 matrix
    MakeXYZCAM(wb, pre_mul, camWbMatrix, preferredIlluminant, mXYZCAM);
    HSBModify *deleteTableHandle;
    const HSBModify *deltaBase = MakeHueSatMap(wb, preferredIlluminant, &deleteTableHandle);

    if (!deltaBase) {
        applyHueSatMap = false;
    }

    if (!look_table) {
        applyLookTable = false;
    }

    useToneCurve &= tone_curve;

    if (!applyHueSatMap && !applyLookTable && !useToneCurve) {
        //===== The fast path: no LUT and not tone curve- Calculate matrix for direct conversion raw>working space
        double mat[3][3] = {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}};

        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 3; j++)
                for (int k = 0; k < 3; k++) {
                    mat[i][j] += mWork[i][k] * mXYZCAM[k][j];
                }

        // Apply the matrix part
        #pragma omp parallel for

        for (int y = 0; y < pImg->height; y++) {
            float newr, newg, newb;

            for (int x = 0; x < pImg->width; x++) {
                newr = mat[0][0] * pImg->r(y, x) + mat[0][1] * pImg->g(y, x) + mat[0][2] * pImg->b(y, x);
                newg = mat[1][0] * pImg->r(y, x) + mat[1][1] * pImg->g(y, x) + mat[1][2] * pImg->b(y, x);
                newb = mat[2][0] * pImg->r(y, x) + mat[2][1] * pImg->g(y, x) + mat[2][2] * pImg->b(y, x);

                pImg->r(y, x) = newr;
                pImg->g(y, x) = newg;
                pImg->b(y, x) = newb;
            }
        }
    } else {
        //===== LUT available- Calculate matrix for conversion raw>ProPhoto
        double m2ProPhoto[3][3] = {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}};

        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 3; j++)
                for (int k = 0; k < 3; k++) {
                    m2ProPhoto[i][j] += prophoto_xyz[i][k] * mXYZCAM[k][j];
                }

        double m2Work[3][3] = {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}};

        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 3; j++)
                for (int k = 0; k < 3; k++) {
                    m2Work[i][j] += mWork[i][k] * xyz_prophoto[k][j];
                }

        // Convert to prophoto and apply LUT
        #pragma omp parallel for

        for (int y = 0; y < pImg->height; y++) {
            float newr, newg, newb, h, s, v, hs, ss, vs;

            for (int x = 0; x < pImg->width; x++) {
                newr = m2ProPhoto[0][0] * pImg->r(y, x) + m2ProPhoto[0][1] * pImg->g(y, x) + m2ProPhoto[0][2] * pImg->b(y, x);
                newg = m2ProPhoto[1][0] * pImg->r(y, x) + m2ProPhoto[1][1] * pImg->g(y, x) + m2ProPhoto[1][2] * pImg->b(y, x);
                newb = m2ProPhoto[2][0] * pImg->r(y, x) + m2ProPhoto[2][1] * pImg->g(y, x) + m2ProPhoto[2][2] * pImg->b(y, x);

                // if point is in negative area, just the matrix, but not the LUT
                if ((applyHueSatMap || applyLookTable) && newr >= 0 && newg >= 0 && newb >= 0) {
                    Color::rgb2hsv(newr, newg, newb, h , s, v);
                    h *= 6.f; // RT calculates in [0,1]

                    if (applyHueSatMap) {
                        HSDApply(delta_info, deltaBase, h, s, v);
                    }

                    if (applyLookTable) {
                        HSDApply(look_info, look_table, h, s, v);
                    }

                    // RT range correction
                    if (h < 0.0f) {
                        h += 6.0f;
                    }

                    if (h >= 6.0f) {
                        h -= 6.0f;
                    }

                    h /= 6.f;
                    Color::hsv2rgb( h, s, v, newr, newg, newb);
                }

                // tone curve
                if (useToneCurve) {
                    tone_curve.Apply(newr, newg, newb);
                }

                pImg->r(y, x) = m2Work[0][0] * newr + m2Work[0][1] * newg + m2Work[0][2] * newb;
                pImg->g(y, x) = m2Work[1][0] * newr + m2Work[1][1] * newg + m2Work[1][2] * newb;
                pImg->b(y, x) = m2Work[2][0] * newr + m2Work[2][1] * newg + m2Work[2][2] * newb;
            }
        }
    }

    if (deleteTableHandle) {
        delete[] deleteTableHandle;
    }
}

void DCPProfile::setStep2ApplyState(const Glib::ustring &workingSpace, bool useToneCurve, bool applyLookTable, bool applyBaselineExposure, ApplyState &asOut)
{

    asOut.useToneCurve = useToneCurve;
    asOut.applyLookTable = applyLookTable;
    asOut.blScale = 1.0;

    if (!look_table) {
        asOut.applyLookTable = false;
    }

    if (!has_tone_curve) {
        asOut.useToneCurve = false;
    }

    if (has_baseline_exposure_offset && applyBaselineExposure) {
        asOut.blScale = powf(2, baseline_exposure_offset);
    }

    if (workingSpace == "ProPhoto") {
        asOut.alreadyProPhoto = true;
    } else {
        asOut.alreadyProPhoto = false;
        TMatrix mWork;

        mWork = iccStore->workingSpaceMatrix (workingSpace);
        memset(asOut.m2ProPhoto, 0, sizeof(asOut.m2ProPhoto));

        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 3; j++)
                for (int k = 0; k < 3; k++) {
                    asOut.m2ProPhoto[i][j] += prophoto_xyz[i][k] * mWork[k][j];
                }

        mWork = iccStore->workingSpaceInverseMatrix (workingSpace);
        memset(asOut.m2Work, 0, sizeof(asOut.m2Work));

        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 3; j++)
                for (int k = 0; k < 3; k++) {
                    asOut.m2Work[i][j] += mWork[i][k] * xyz_prophoto[k][j];
                }
    }
}

void DCPProfile::step2ApplyTile(float *rc, float *gc, float *bc, int width, int height, int tileWidth, const ApplyState &asIn) const
{

#define FCLIP(a) ((a)>0.0?((a)<65535.5?(a):65535.5):0.0)
#define CLIP01(a) ((a)>0?((a)<1?(a):1):0)

    float exp_scale = 1.0;
    exp_scale *= asIn.blScale;

    if (!asIn.useToneCurve && !asIn.applyLookTable) {
        if (exp_scale == 1.0) {
            return;
        }

        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                rc[y * tileWidth + x] *= exp_scale;
                gc[y * tileWidth + x] *= exp_scale;
                bc[y * tileWidth + x] *= exp_scale;
            }
        }
    } else {
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                float r = rc[y * tileWidth + x];
                float g = gc[y * tileWidth + x];
                float b = bc[y * tileWidth + x];

                if (exp_scale != 1.0) {
                    r *= exp_scale;
                    g *= exp_scale;
                    b *= exp_scale;
                }

                float newr, newg, newb;

                if (asIn.alreadyProPhoto) {
                    newr = r;
                    newg = g;
                    newb = b;
                } else {
                    newr = asIn.m2ProPhoto[0][0] * r + asIn.m2ProPhoto[0][1] * g + asIn.m2ProPhoto[0][2] * b;
                    newg = asIn.m2ProPhoto[1][0] * r + asIn.m2ProPhoto[1][1] * g + asIn.m2ProPhoto[1][2] * b;
                    newb = asIn.m2ProPhoto[2][0] * r + asIn.m2ProPhoto[2][1] * g + asIn.m2ProPhoto[2][2] * b;
                }

                // with looktable and tonecurve we need to clip
                newr = FCLIP(newr);
                newg = FCLIP(newg);
                newb = FCLIP(newb);

                if (asIn.applyLookTable) {
                    float h, s, v;
                    Color::rgb2hsv(newr, newg, newb, h, s, v);
                    h *= 6.f; // RT calculates in [0,1]

                    HSDApply(look_info, look_table, h, s, v);
                    s = CLIP01(s);
                    v = CLIP01(v);

                    // RT range correction
                    if (h < 0.0f) {
                        h += 6.0f;
                    }

                    if (h >= 6.0f) {
                        h -= 6.0f;
                    }

                    h /= 6.f;
                    Color::hsv2rgb( h, s, v, newr, newg, newb);
                }

                if (asIn.useToneCurve) {
                    tone_curve.Apply(newr, newg, newb);
                }

                if (asIn.alreadyProPhoto) {
                    rc[y * tileWidth + x] = newr;
                    gc[y * tileWidth + x] = newg;
                    bc[y * tileWidth + x] = newb;
                } else {
                    rc[y * tileWidth + x] = asIn.m2Work[0][0] * newr + asIn.m2Work[0][1] * newg + asIn.m2Work[0][2] * newb;
                    gc[y * tileWidth + x] = asIn.m2Work[1][0] * newr + asIn.m2Work[1][1] * newg + asIn.m2Work[1][2] * newb;
                    bc[y * tileWidth + x] = asIn.m2Work[2][0] * newr + asIn.m2Work[2][1] * newg + asIn.m2Work[2][2] * newb;
                }
            }
        }
    }
}

void DCPProfile::dngref_XYCoord2Temperature(const double whiteXY[2], double *temp, double *tint) const
{
    constexpr double kTintScale = -3000.0;

    double fTemperature = 0;
    double fTint = 0;

    // Convert to uv space.
    double u = 2.0 * whiteXY[0] / (1.5 - whiteXY[0] + 6.0 * whiteXY[1]);
    double v = 3.0 * whiteXY[1] / (1.5 - whiteXY[0] + 6.0 * whiteXY[1]);

    // Search for line pair coordinate is between.
    double last_dt = 0.0;
    double last_dv = 0.0;
    double last_du = 0.0;

    for (uint32_t index = 1; index <= 30; index++) {
        // Convert slope to delta-u and delta-v, with length 1.
        double du = 1.0;
        double dv = kTempTable [index] . t;
        double len = sqrt (1.0 + dv * dv);
        du /= len;
        dv /= len;

        // Find delta from black body point to test coordinate.
        double uu = u - kTempTable [index] . u;
        double vv = v - kTempTable [index] . v;

        // Find distance above or below line.
        double dt = - uu * dv + vv * du;

        // If below line, we have found line pair.
        if (dt <= 0.0 || index == 30) {
            // Find fractional weight of two lines.
            if (dt > 0.0) {
                dt = 0.0;
            }

            dt = -dt;
            double f;

            if (index == 1) {
                f = 0.0;
            } else {
                f = dt / (last_dt + dt);
            }

            // Interpolate the temperature.
            fTemperature = 1.0E6 / (kTempTable [index - 1] . r * f +
                                    kTempTable [index    ] . r * (1.0 - f));

            // Find delta from black body point to test coordinate.
            uu = u - (kTempTable [index - 1] . u * f +
                      kTempTable [index    ] . u * (1.0 - f));
            vv = v - (kTempTable [index - 1] . v * f +
                      kTempTable [index    ] . v * (1.0 - f));
            // Interpolate vectors along slope.
            du = du * (1.0 - f) + last_du * f;
            dv = dv * (1.0 - f) + last_dv * f;
            len = sqrt (du * du + dv * dv);
            du /= len;
            dv /= len;

            // Find distance along slope.
            fTint = (uu * du + vv * dv) * kTintScale;
            break;
        }

        // Try next line pair.
        last_dt = dt;
        last_du = du;
        last_dv = dv;
    }

    if (temp != nullptr) {
        *temp = fTemperature;
    }

    if (tint != nullptr) {
        *tint = fTint;
    }
}

void DCPProfile::dngref_FindXYZtoCamera(const double whiteXY[2], int preferredIlluminant, double (*xyzToCamera)[3]) const
{

    bool hasCol1 = has_color_matrix_1;
    bool hasCol2 = has_color_matrix_2;

    if (preferredIlluminant == 1) {
        if (hasCol1) {
            hasCol2 = false;
        }
    } else if (preferredIlluminant == 2) {
        if (hasCol2) {
            hasCol1 = false;
        }
    }

    // mix if we have two matrices
    double mix;

    if (hasCol1 && hasCol2) {
        double wbtemp;
        /*
          Note: we're using DNG SDK reference code for XY to temperature translation to get the exact same mix as
          the reference code does.
        */
        dngref_XYCoord2Temperature(whiteXY, &wbtemp, nullptr);

        if (wbtemp <= temperature_1) {
            mix = 1.0;
        } else if (wbtemp >= temperature_2) {
            mix = 0.0;
        } else {
            double invT = 1.0 / wbtemp;
            mix = (invT - (1.0 / temperature_2)) / ((1.0 / temperature_1) - (1.0 / temperature_2));
        }
    }

    // Interpolate the color matrix.
    double mCol[3][3];

    if (hasCol1 && hasCol2) {
        // interpolate
        if (mix >= 1.0) {
            memcpy(mCol, color_matrix_1, sizeof(mCol));
        } else if (mix <= 0.0) {
            memcpy(mCol, color_matrix_2, sizeof(mCol));
        } else {
            mix3x3(color_matrix_1, mix, color_matrix_2, 1.0 - mix, mCol);
        }
    } else if (hasCol1) {
        memcpy(mCol, color_matrix_1, sizeof(mCol));
    } else {
        memcpy(mCol, color_matrix_2, sizeof(mCol));
    }

    memcpy(xyzToCamera, mCol, sizeof(mCol));
}

void DCPProfile::dngref_NeutralToXY(double neutral[3], int preferredIlluminant, double XY[2]) const
{
    const int kMaxPasses = 30;
    double lastXY[2] = { 0.3457, 0.3585 }; // D50

    for (int pass = 0; pass < kMaxPasses; pass++) {
        double xyzToCamera[3][3];
        dngref_FindXYZtoCamera(lastXY, preferredIlluminant, xyzToCamera);

        double invM[3][3], nextXYZ[3], nextXY[2];
        invert3x3(xyzToCamera, invM);
        multiply3x3_v3(invM, neutral, nextXYZ);
        XYZtoXY(nextXYZ, nextXY);

        if (fabs(nextXY[0] - lastXY[0]) +
                fabs(nextXY[1] - lastXY[1]) < 0.0000001) {
            XY[0] = nextXY[0];
            XY[1] = nextXY[1];
            return;
        }

        // If we reach the limit without converging, we are most likely
        // in a two value oscillation.  So take the average of the last
        // two estimates and give up.
        if (pass == kMaxPasses - 1) {
            nextXY[0] = (lastXY[0] + nextXY[0]) * 0.5;
            nextXY[1] = (lastXY[1] + nextXY[1]) * 0.5;
        }

        lastXY[0] = nextXY[0];
        lastXY[1] = nextXY[1];
    }

    XY[0] = lastXY[0];
    XY[1] = lastXY[1];
}

void DCPProfile::MakeXYZCAM(const ColorTemp &wb, double pre_mul[3], double camWbMatrix[3][3], int preferredIlluminant, double (*mXYZCAM)[3]) const
{
    // code adapted from dng_color_spec::FindXYZtoCamera
    // note that we do not support monochrome or colorplanes > 3 (no reductionMatrix support)
    // we do not support cameracalibration either

    double neutral[3]; // same as the DNG "AsShotNeutral" tag if white balance is Camera's own
    {
        /* A bit messy matrixing and conversions to get the neutral[] array from RT's own white balance which is stored in
           sRGB space, while the DCP code needs multipliers in CameraRGB space */
        double r, g, b;
        wb.getMultipliers(r, g, b);

        // camWbMatrix == imatrices.xyz_cam
        double cam_xyz[3][3];
        invert3x3(camWbMatrix, cam_xyz);
        double cam_rgb[3][3];
        multiply3x3(cam_xyz, xyz_sRGB, cam_rgb);
        double camwb_red   = cam_rgb[0][0] * r + cam_rgb[0][1] * g + cam_rgb[0][2] * b;
        double camwb_green = cam_rgb[1][0] * r + cam_rgb[1][1] * g + cam_rgb[1][2] * b;
        double camwb_blue  = cam_rgb[2][0] * r + cam_rgb[2][1] * g + cam_rgb[2][2] * b;
        neutral[0] = camwb_red / pre_mul[0];
        neutral[1] = camwb_green / pre_mul[1];
        neutral[2] = camwb_blue / pre_mul[2];
        double maxentry = 0;

        for (int i = 0; i < 3; i++) {
            if (neutral[i] > maxentry) {
                maxentry = neutral[i];
            }
        }

        for (int i = 0; i < 3; i++) {
            neutral[i] /= maxentry;
        }
    }

    /* Calculate what the RGB multipliers corresponds to as a white XY coordinate, based on the
       DCP ColorMatrix or ColorMatrices if dual-illuminant. This is the DNG reference code way to
       do it, which is a bit different from RT's own white balance model at the time of writing.
       When RT's white balance can make use of the DCP color matrices we could use that instead. */
    double white_xy[2];
    dngref_NeutralToXY(neutral, preferredIlluminant, white_xy);

    bool hasFwd1 = has_forward_matrix_1;
    bool hasFwd2 = has_forward_matrix_2;
    bool hasCol1 = has_color_matrix_1;
    bool hasCol2 = has_color_matrix_2;

    if (preferredIlluminant == 1) {
        if (hasFwd1) {
            hasFwd2 = false;
        }

        if (hasCol1) {
            hasCol2 = false;
        }
    } else if (preferredIlluminant == 2) {
        if (hasFwd2) {
            hasFwd1 = false;
        }

        if (hasCol2) {
            hasCol1 = false;
        }
    }

    // mix if we have two matrices
    double mix = 1.0;

    if ((hasCol1 && hasCol2) || (hasFwd1 && hasFwd2)) {
        double wbtemp;
        /* DNG ref way to convert XY to temperature, which affect matrix mixing. A different model here
           typically does not affect the result too much, ie it's probably not strictly necessary to
           use the DNG reference code here, but we do it for now. */
        dngref_XYCoord2Temperature(white_xy, &wbtemp, nullptr);

        if (wbtemp <= temperature_1) {
            mix = 1.0;
        } else if (wbtemp >= temperature_2) {
            mix = 0.0;
        } else {
            double invT = 1.0 / wbtemp;
            mix = (invT - (1.0 / temperature_2)) / ((1.0 / temperature_1) - (1.0 / temperature_2));
        }
    }

    // Colormatrix
    double mCol[3][3];

    if (hasCol1 && hasCol2) {
        // interpolate
        if (mix >= 1.0) {
            memcpy(mCol, color_matrix_1, sizeof(mCol));
        } else if (mix <= 0.0) {
            memcpy(mCol, color_matrix_2, sizeof(mCol));
        } else {
            mix3x3(color_matrix_1, mix, color_matrix_2, 1.0 - mix, mCol);
        }
    } else if (hasCol1) {
        memcpy(mCol, color_matrix_1, sizeof(mCol));
    } else {
        memcpy(mCol, color_matrix_2, sizeof(mCol));
    }

    /*
      The exact position of the white XY coordinate affects the result very much, thus
      it's important that the result is very similar or the same as DNG reference code.
      Especially important is it that the raw-embedded "AsShot" multipliers is translated
      to the same white XY coordinate as the DNG reference code, or else third party DCPs
      will show incorrect color.
    */

    double white_xyz[3];
    XYtoXYZ(white_xy, white_xyz);

    double cam_xyz[3][3];

    if (hasFwd1 || hasFwd2) {
        // always prefer ForwardMatrix ahead of ColorMatrix
        double mFwd[3][3];

        if (hasFwd1 && hasFwd2) {
            // interpolate
            if (mix >= 1.0) {
                memcpy(mFwd, forward_matrix_1, sizeof(mFwd));
            } else if (mix <= 0.0) {
                memcpy(mFwd, forward_matrix_2, sizeof(mFwd));
            } else {
                mix3x3(forward_matrix_1, mix, forward_matrix_2, 1.0 - mix, mFwd);
            }
        } else if (hasFwd1) {
            memcpy(mFwd, forward_matrix_1, sizeof(mFwd));
        } else {
            memcpy(mFwd, forward_matrix_2, sizeof(mFwd));
        }

        // adapted from dng_color_spec::SetWhiteXY
        double CameraWhite[3];
        multiply3x3_v3(mCol, white_xyz, CameraWhite);

        double whiteDiag[3][3] = {{CameraWhite[0], 0, 0}, {0, CameraWhite[1], 0}, {0, 0, CameraWhite[2]}};
        double whiteDiagInv[3][3];
        invert3x3(whiteDiag, whiteDiagInv);

        double xyz_cam[3][3];
        multiply3x3(mFwd, whiteDiagInv, xyz_cam);
        invert3x3(xyz_cam, cam_xyz);
    } else {
        double whiteMatrix[3][3];
        const double white_d50[3] = { 0.3457, 0.3585, 0.2958 }; // D50
        mapWhiteMatrix(white_d50, white_xyz, whiteMatrix);
        multiply3x3(mCol, whiteMatrix, cam_xyz);
    }

    // convert cam_xyz (XYZ D50 to CameraRGB, "PCS to Camera" in DNG terminology) to mXYZCAM

    {
        // This block can probably be simplified, seems unnecessary to pass through the sRGB matrix
        // (probably dcraw legacy), it does no harm though as we don't clip anything.
        int i, j, k;

        // Multiply out XYZ colorspace
        double cam_rgb[3][3] = {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}};

        for (i = 0; i < 3; i++)
            for (j = 0; j < 3; j++)
                for (k = 0; k < 3; k++) {
                    cam_rgb[i][j] += cam_xyz[i][k] * xyz_sRGB[k][j];
                }

        // Normalize cam_rgb so that:  cam_rgb * (1,1,1) is (1,1,1,1)
        double num;

        for (i = 0; i < 3; i++) {
            for (num = j = 0; j < 3; j++) {
                num += cam_rgb[i][j];
            }

            for (j = 0; j < 3; j++) {
                cam_rgb[i][j] /= num;
            }
        }

        double rgb_cam[3][3] = {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}};
        RawImageSource::inverse33 (cam_rgb, rgb_cam);

        for (i = 0; i < 3; i++)
            for (j = 0; j < 3; j++) {
                mXYZCAM[i][j] = 0;
            }

        for (i = 0; i < 3; i++)
            for (j = 0; j < 3; j++)
                for (k = 0; k < 3; k++) {
                    mXYZCAM[i][j] += xyz_sRGB[i][k] * rgb_cam[k][j];
                }
    }
}

const DCPProfile::HSBModify* DCPProfile::MakeHueSatMap(const ColorTemp &wb, int preferredIlluminant, HSBModify **deleteHandle) const
{

    *deleteHandle = nullptr;

    if (!deltas_1) {
        return nullptr;
    }

    if (!deltas_2) {
        return deltas_1;
    }

    if (preferredIlluminant == 1) {
        return deltas_1;
    } else if (preferredIlluminant == 2) {
        return deltas_2;
    }

    // Interpolate based on color temperature.
    if (temperature_1 <= 0.0 || temperature_2 <= 0.0 || temperature_1 == temperature_2) {
        return deltas_1;
    }

    bool reverseOrder = temperature_1 > temperature_2;
    double t1, t2;

    if (reverseOrder) {
        t1 = temperature_2;
        t2 = temperature_1;
    } else {
        t1 = temperature_1;
        t2 = temperature_2;
    }

    double mix;

    if (wb.getTemp() <= t1) {
        mix = 1.0;
    } else if (wb.getTemp() >= t2) {
        mix = 0.0;
    } else {
        double invT = 1.0 / wb.getTemp();
        mix = (invT - (1.0 / t2)) / ((1.0 / t1) - (1.0 / t2));
    }

    if (reverseOrder) {
        mix = 1.0 - mix;
    }

    if (mix >= 1.0) {
        return deltas_1;
    } else if (mix <= 0.0) {
        return deltas_2;
    }

    // Interpolate between the tables.
    HSBModify *aDeltas = new HSBModify[delta_info.array_count];
    *deleteHandle = aDeltas;
    float w1 = (float)mix;
    float w2 = 1.0f - (float)mix;

    for (int i = 0; i < delta_info.array_count; i++) {
        aDeltas[i].hue_shift = w1 * deltas_1[i].hue_shift + w2 * deltas_2[i].hue_shift;
        aDeltas[i].sat_scale = w1 * deltas_1[i].sat_scale + w2 * deltas_2[i].sat_scale;
        aDeltas[i].val_scale = w1 * deltas_1[i].val_scale + w2 * deltas_2[i].val_scale;
    }

    return aDeltas;
}

void DCPProfile::HSDApply(const HSDTableInfo &ti, const HSBModify *tableBase, float &h, float &s, float &v) const
{

    // Apply the HueSatMap. Ported from Adobes reference implementation
    float hueShift, satScale, valScale;
    float vEncoded = v;

    if (ti.val_divisions < 2) { // Optimize most common case of "2.5D" table.
        float hScaled = h * ti.pc.h_scale;
        float sScaled = s * ti.pc.s_scale;

        int hIndex0 = max((int)hScaled, 0);
        int sIndex0 = max(min((int)sScaled, ti.pc.maxSatIndex0), 0);

        int hIndex1 = hIndex0 + 1;

        if (hIndex0 >= ti.pc.maxHueIndex0) {
            hIndex0 = ti.pc.maxHueIndex0;
            hIndex1 = 0;
        }

        float hFract1 = hScaled - (float) hIndex0;
        float sFract1 = sScaled - (float) sIndex0;

        float hFract0 = 1.0f - hFract1;
        float sFract0 = 1.0f - sFract1;

        const HSBModify *entry00 = tableBase + hIndex0 * ti.pc.hueStep + sIndex0;
        const HSBModify *entry01 = entry00 + (hIndex1 - hIndex0) * ti.pc.hueStep;

        float hueShift0 = hFract0 * entry00->hue_shift + hFract1 * entry01->hue_shift;
        float satScale0 = hFract0 * entry00->sat_scale + hFract1 * entry01->sat_scale;
        float valScale0 = hFract0 * entry00->val_scale + hFract1 * entry01->val_scale;

        entry00++;
        entry01++;

        float hueShift1 = hFract0 * entry00->hue_shift +
                          hFract1 * entry01->hue_shift;

        float satScale1 = hFract0 * entry00->sat_scale +
                          hFract1 * entry01->sat_scale;

        float valScale1 = hFract0 * entry00->val_scale +
                          hFract1 * entry01->val_scale;

        hueShift = sFract0 * hueShift0 + sFract1 * hueShift1;
        satScale = sFract0 * satScale0 + sFract1 * satScale1;
        valScale = sFract0 * valScale0 + sFract1 * valScale1;

    } else {

        float hScaled = h * ti.pc.h_scale;
        float sScaled = s * ti.pc.s_scale;

        if (ti.srgb_gamma) {
            vEncoded = sRGBGammaForward(v);
        }

        float vScaled = vEncoded * ti.pc.v_scale;

        int hIndex0 = (int) hScaled;
        int sIndex0 = max(min((int)sScaled, ti.pc.maxSatIndex0), 0);
        int vIndex0 = max(min((int)vScaled, ti.pc.maxValIndex0), 0);

        int hIndex1 = hIndex0 + 1;

        if (hIndex0 >= ti.pc.maxHueIndex0) {
            hIndex0 = ti.pc.maxHueIndex0;
            hIndex1 = 0;
        }

        float hFract1 = hScaled - (float) hIndex0;
        float sFract1 = sScaled - (float) sIndex0;
        float vFract1 = vScaled - (float) vIndex0;

        float hFract0 = 1.0f - hFract1;
        float sFract0 = 1.0f - sFract1;
        float vFract0 = 1.0f - vFract1;

        const HSBModify *entry00 = tableBase + vIndex0 * ti.pc.valStep + hIndex0 * ti.pc.hueStep + sIndex0;

        const HSBModify *entry01 = entry00 + (hIndex1 - hIndex0) * ti.pc.hueStep;

        const HSBModify *entry10 = entry00 + ti.pc.valStep;
        const HSBModify *entry11 = entry01 + ti.pc.valStep;

        float hueShift0 = vFract0 * (hFract0 * entry00->hue_shift +
                                     hFract1 * entry01->hue_shift) +
                          vFract1 * (hFract0 * entry10->hue_shift +
                                     hFract1 * entry11->hue_shift);

        float satScale0 = vFract0 * (hFract0 * entry00->sat_scale +
                                     hFract1 * entry01->sat_scale) +
                          vFract1 * (hFract0 * entry10->sat_scale +
                                     hFract1 * entry11->sat_scale);

        float valScale0 = vFract0 * (hFract0 * entry00->val_scale +
                                     hFract1 * entry01->val_scale) +
                          vFract1 * (hFract0 * entry10->val_scale +
                                     hFract1 * entry11->val_scale);

        entry00++;
        entry01++;
        entry10++;
        entry11++;

        float hueShift1 = vFract0 * (hFract0 * entry00->hue_shift +
                                     hFract1 * entry01->hue_shift) +
                          vFract1 * (hFract0 * entry10->hue_shift +
                                     hFract1 * entry11->hue_shift);

        float satScale1 = vFract0 * (hFract0 * entry00->sat_scale +
                                     hFract1 * entry01->sat_scale) +
                          vFract1 * (hFract0 * entry10->sat_scale +
                                     hFract1 * entry11->sat_scale);

        float valScale1 = vFract0 * (hFract0 * entry00->val_scale +
                                     hFract1 * entry01->val_scale) +
                          vFract1 * (hFract0 * entry10->val_scale +
                                     hFract1 * entry11->val_scale);

        hueShift = sFract0 * hueShift0 + sFract1 * hueShift1;
        satScale = sFract0 * satScale0 + sFract1 * satScale1;
        valScale = sFract0 * valScale0 + sFract1 * valScale1;
    }

    hueShift *= (6.0f / 360.0f);    // Convert to internal hue range.

    h += hueShift;
    s *= satScale;  // no clipping here, we are RT float :-)

    if (ti.srgb_gamma) {
        v = sRGBGammaInverse(vEncoded * valScale);
    } else {
        v *= valScale;
    }
}

DCPStore* DCPStore::getInstance()
{
    static DCPStore instance;
    return &instance;
}

void DCPStore::init(const Glib::ustring& rt_profile_dir)
{
    MyMutex::MyLock lock(mutex);

    file_std_profiles.clear();

    if (!rt_profile_dir.empty()) {
        std::deque<Glib::ustring> dirs = {
            rt_profile_dir
        };

        while (!dirs.empty()) {
            // Process directory
            Glib::ustring dirname = dirs.back();
            dirs.pop_back();

            std::unique_ptr<Glib::Dir> dir;

            try {
                if (!Glib::file_test(dirname, Glib::FILE_TEST_IS_DIR)) {
                    return;
                }

                dir.reset(new Glib::Dir(dirname));
            } catch (Glib::Exception& exception) {
                return;
            }

            dirname += '/';

            for (const Glib::ustring& sname : *dir) {
                const Glib::ustring fname = dirname + sname;

                if (!Glib::file_test(fname, Glib::FILE_TEST_IS_DIR)) {
                    // File
                    const auto lastdot = sname.rfind('.');

                    if (
                        lastdot != Glib::ustring::npos
                        && lastdot <= sname.size() - 4
                        && !sname.casefold().compare(lastdot, 4, ".dcp")
                    ) {
                        const Glib::ustring cam_short_name = sname.substr(0, lastdot).uppercase();
                        file_std_profiles[cam_short_name] = fname; // They will be loaded and cached on demand
                    }
                } else {
                    // Directory
                    dirs.push_front(fname);
                }
            }
        }
    }
}

bool DCPStore::isValidDCPFileName(const Glib::ustring& filename) const
{
    if (!Glib::file_test(filename, Glib::FILE_TEST_EXISTS) || Glib::file_test(filename, Glib::FILE_TEST_IS_DIR)) {
        return false;
    }

    const auto pos = filename.rfind('.');
    return
        pos > 0
        && (
            !filename.casefold().compare(pos, 4, ".dcp")
            || !filename.casefold().compare(pos, 4, ".dng")
        );
}

DCPProfile* DCPStore::getProfile(const Glib::ustring& filename) const
{
    MyMutex::MyLock lock(mutex);

    const std::map<Glib::ustring, DCPProfile*>::iterator r = profile_cache.find(filename);

    if (r != profile_cache.end()) {
        return r->second;
    }

    DCPProfile* const res = new DCPProfile(filename);

    // Add profile
    profile_cache[filename] = res;

    return res;
}

DCPProfile* DCPStore::getStdProfile(const Glib::ustring& cam_short_name) const
{
    const Glib::ustring name = cam_short_name.uppercase();

    // Warning: do NOT use map.find(), since it does not seem to work reliably here
    for (const auto& file_std_profile : file_std_profiles)
        if (file_std_profile.first == name) {
            return getProfile(file_std_profile.second);
        }

    return nullptr;
}
