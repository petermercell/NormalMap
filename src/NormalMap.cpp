/*

    NormalMap.cpp: (another) very useful Nuke plugin

    Copyright (C) 2011  Jonathan Egstad

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <DDImage/Iop.h>
#include <DDImage/Row.h>
#include <DDImage/Interest.h>
#include <DDImage/Knobs.h>
#include <DDImage/Vector3.h>

using namespace DD::Image;

/*!
*/
class NormalMap : public Iop {
   Channel k_source_chan;           //!< Input depth channel
   float   k_gain;                  //!< Normal gain
   Channel k_normal_chans[3];       //!< Normal output channels
   bool    k_invert_x;              //!< Invert N.x
   bool    k_invert_y;              //!< Invert N.y
   bool    k_invert_z;              //!< Invert N.z
   bool    k_output_zero_to_one;    //!< Output normal centered around 0.5

   ChannelSet m_input_chans;        //!< Set of channels we read from
   ChannelSet m_output_chans;       //!< Set of channels we write to

public:
   static const Description description;
   /*virtual*/ const char* Class() const { return description.name; }
   /*virtual*/ const char* node_help() const { return  __DATE__ " " __TIME__ "\n"
      "NormalMap accepts a depth channel (usually a heightfield texturemap) "
      "and outputs a normal field based on perceived contours in the image.\n"
      "\n"
      "Uses a Sobel filter to determine the contour.";
      "\n"
      "NormalMap  Copyright (C) 2011  Jonathan Egstad\n"
      "This program comes with ABSOLUTELY NO WARRANTY;\n"
      "This is free software, and you are welcome to redistribute it under certain conditions;";
    }

   NormalMap(Node* node) : Iop(node) {
      k_source_chan = Chan_Red;
      k_gain = 1.0f;
      for (int z=0; z < 3; ++z)
         k_normal_chans[z] = Channel(Chan_Red + z);
      k_invert_x = k_invert_y = k_invert_z = false;
      k_output_zero_to_one = false;
   }

   /*virtual*/
   void _validate(bool for_real) {
      copy_info();

      // Build our input/output channel masks:
      m_input_chans.clear();
      m_input_chans += k_source_chan;
      m_output_chans.clear();
      for (int z=0; z < 3; ++z)
        m_output_chans += k_normal_chans[z];

      // Declare our writable output channels:
	   info_.turn_on(m_output_chans);
   }

   /*virtual*/
   void _request(int x, int y, int r, int t, ChannelMask channels, int count) {
	   // Request source channel:
	   ChannelSet c1(channels);
	   c1 += m_input_chans;
      // Expand the input area by the filter size:
	   input0().request(x-1, y-1, r+1, t+1, c1, count+1);
   }

   /*virtual*/
   void knobs(Knob_Callback f) {
      Input_Channel_knob(f, &k_source_chan, 1, 0, "source_chan", "source channel");
      Tooltip(f, "Which channel to use for the contour filter source.");
      Newline(f);
      Float_knob(f, &k_gain, IRange(0.1, 100.0), "gain");
      Newline(f);
      Channel_knob(f, k_normal_chans, 3, "normal", "output normal");
      Tooltip(f, "Which channels to write the derived normal to.");
      Newline(f);
      Bool_knob(f, &k_invert_x, "invert_x", "x");
      Bool_knob(f, &k_invert_y, "invert_y", "y");
      Bool_knob(f, &k_invert_z, "invert_z", "z   invert");
      Newline(f);
      Bool_knob(f, &k_output_zero_to_one, "zero_to_one", "zero to one");
      Tooltip(f, "If on, output normal values go from 0-1, centered on 0.5.");
   }

   /*virtual*/
   void engine(int y, int x, int r, ChannelMask channels, Row& row) {
      // Copy our input:
      input0().get(y, x, r, channels, row);

      int X = x - 1;
      const int R = r + 1;

      // Read our input tile:
      Interest interest(input0(), X, y-1, R, y+1, m_input_chans);
      Row in0(X, R);
      input0().get(y+1, X, R, channels, in0);
      Row in1(X, R);
      input0().get(y  , X, R, channels, in1);
      Row in2(X, R);
      input0().get(y-1, X, R, channels, in2);

      float* OUTR = row.writable(k_normal_chans[0])+x;
      float* OUTG = row.writable(k_normal_chans[1])+x;
      float* OUTB = row.writable(k_normal_chans[2])+x;
      const float* END = OUTR+(r-x);
      while (OUTR < END) {
         Vector3 N(0.0f, 0.0f, 1.0f);
         //
         // Do X & Y Sobel filter:
         const float* p;
         p = in0[k_source_chan]+X; // upper row
         N.y += *p *  1.0f*k_gain; N.x += *p *  1.0f*k_gain; p++;
         N.y += *p *  2.0f*k_gain; p++;
         N.y += *p *  1.0f*k_gain; N.x += *p * -1.0f*k_gain;
         p = in1[k_source_chan]+X; // center row
         N.x += *p *  2.0f*k_gain; p += 2;
         N.x += *p * -2.0f*k_gain;
         p = in2[k_source_chan]+X; // lower row
         N.y += *p * -1.0f*k_gain; N.x += *p *  1.0f*k_gain; p++;
         N.y += *p * -2.0f*k_gain; p++;
         N.y += *p * -1.0f*k_gain; N.x += *p * -1.0f*k_gain;
         //
         // Cross-product of components of gradient reduces to:
         N.normalize();
         if (k_invert_x) N.x = -N.x;
         if (k_invert_y) N.y = -N.y;
         if (k_invert_z) N.z = -N.z;
         //
         // Output either raw normal or scaled into 0-1:
         if (k_output_zero_to_one) {
            *OUTR++ = N.x/2.0f + 0.5f;
            *OUTG++ = N.y/2.0f + 0.5f;
            *OUTB++ = N.z/2.0f + 0.5f;
         } else {
            *OUTR++ = N.x;
            *OUTG++ = N.y;
            *OUTB++ = N.z;
         }
         ++X;
      }
   }

};

static Op* build(Node* node) { return new NormalMap(node); }
const Op::Description NormalMap::description("NormalMap", build);

// end of NormalMap.cpp
