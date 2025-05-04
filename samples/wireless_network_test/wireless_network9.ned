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
simple box9a
{
    parameters:
        @display("i=block/rxtx");
    gates:
        input in;
        output out;
}

simple box9b
{
    parameters:
        @display("i=block/rxtx");
    gates:
        input in;
        output out;
}

network wn9
{
    @display("bgb=354,108");
    submodules:
        box9a: box9a{
        parameters:
	        @display("i=,cyan");      
        };
        box9b: box9b{
        parameters:
	        @display("i=,red");        
        };
    connections:
        box9a.out --> {delay = 100ms;} --> box9b.in;
        box9b.out --> {delay = 100ms;} --> box9a.in;
}