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
simple box3
{
    parameters:
        @display("i=block/routing");
    gates:
        input in;
        output out;
}

network wn3
{
    @display("bgb=354,108");
    submodules:
        box3a: box3{
        parameters:
	        @display("i=,cyan");        
        };
        box3b: box3{
        parameters:
	        @display("i=,red");        
        };
    connections:
        box3a.out --> {delay = 100ms;} --> box3b.in;
        box3b.out --> {delay = 100ms;} --> box3a.in;
}