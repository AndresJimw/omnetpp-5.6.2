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
simple box11
{
    parameters:
        @display("i=block/rxtx");
    gates:
        input in[];
        output out[];
}

network wn11
{
    @display("bgb=354,108");
    submodules:
        box11[6]: box11;
    connections:
        box11[0].out++ --> {delay = 100ms;} --> box11[1].in++;
        box11[1].out++ --> {delay = 100ms;} --> box11[0].in++;
        box11[1].out++ --> {delay = 100ms;} --> box11[2].in++;
        box11[2].out++ --> {delay = 100ms;} --> box11[1].in++;
        box11[1].out++ --> {delay = 100ms;} --> box11[4].in++;
        box11[4].out++ --> {delay = 100ms;} --> box11[1].in++;
        box11[3].out++ --> {delay = 100ms;} --> box11[4].in++;
        box11[4].out++ --> {delay = 100ms;} --> box11[3].in++;
        box11[4].out++ --> {delay = 100ms;} --> box11[5].in++;
        box11[5].out++ --> {delay = 100ms;} --> box11[4].in++;
        
}