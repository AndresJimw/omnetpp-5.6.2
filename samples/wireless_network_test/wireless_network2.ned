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
simple box2
{
    gates:
        input in;
        output out;
}

network wn2
{
    submodules:
        box2a: box2;
        box2b: box2;
    connections:
        box2a.out --> {delay = 100ms;} --> box2b.in;
        box2b.out --> {delay = 100ms;} --> box2a.in;
}