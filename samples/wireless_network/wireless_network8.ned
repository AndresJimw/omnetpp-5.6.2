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
simple box8a
{
    parameters:
        @display("i=block/routing");
    gates:
        input in;
        output out;
}

simple box8b
{
    parameters:
        @display("i=block/routing");
    gates:
        input in;
        output out;
}

network wn8
{
    @display("bgb=354,108");
    submodules:
        box8a: box8a{
        parameters:
	        @display("i=,cyan");      
        };
        box8b: box8b{
        parameters:
	        @display("i=,red");        
        };
    connections:
        box8a.out --> {delay = 100ms;} --> box8b.in;
        box8b.out --> {delay = 100ms;} --> box8a.in;
}