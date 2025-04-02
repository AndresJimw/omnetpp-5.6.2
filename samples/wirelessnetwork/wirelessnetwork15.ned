network wn15
{
    types:
        channel canal extends ned.DelayChannel{ delay = 100ms; }
    submodules:
        box15[6]: box15;
    connections:
        box15[0].gate++ <--> canal <--> box15[1].gate++;
        box15[1].gate++ <--> canal <--> box15[2].gate++;
        box15[1].gate++ <--> canal <--> box15[4].gate++;
        box15[3].gate++ <--> canal <--> box15[4].gate++;
        box15[4].gate++ <--> canal <--> box15[5].gate++;
}

simple box15
{
    parameters:
        @display("i=block/rxtx");
    
    gates:
        inout gate[];
}



