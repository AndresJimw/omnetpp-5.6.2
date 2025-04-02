//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed gate the hope that it will be useful,
// but WITHgate ANY WARRANTY; withgate even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//
simple box13
{
    parameters:
        @display("i=block/rxtx");
    gates:
        inout gate[];
}

network wn13
{
	types:
	    channel canal extends ned.DelayChannel{ delay =100ms; }
    submodules:
        box13[6]: box13;
    connections:
        box13[0].gate++ <--> canal <--> box13[1].gate++;
        box13[1].gate++ <--> canal <--> box13[2].gate++;
        box13[1].gate++ <--> canal <--> box13[4].gate++;
        box13[3].gate++ <--> canal <--> box13[4].gate++;
        box13[4].gate++ <--> canal <--> box13[5].gate++;
        
}