//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//
simple box12
{
    parameters:
        @display("i=block/rxtx");
    gates:
        input in[];
        output out[];
}

network wn12
{
    @display("bgb=354,108");
    submodules:
        box12[6]: box12;
    connections:
        box12[0].out++ --> {delay = 100ms;} --> box12[1].in++;
        box12[1].out++ --> {delay = 100ms;} --> box12[0].in++;
        box12[1].out++ --> {delay = 100ms;} --> box12[2].in++;
        box12[2].out++ --> {delay = 100ms;} --> box12[1].in++;
        box12[1].out++ --> {delay = 100ms;} --> box12[4].in++;
        box12[4].out++ --> {delay = 100ms;} --> box12[1].in++;
        box12[3].out++ --> {delay = 100ms;} --> box12[4].in++;
        box12[4].out++ --> {delay = 100ms;} --> box12[3].in++;
        box12[4].out++ --> {delay = 100ms;} --> box12[5].in++;
        box12[5].out++ --> {delay = 100ms;} --> box12[4].in++;
        
}