network wn16
{
    types:
        channel canal extends ned.DelayChannel{ delay = 100ms; }
    submodules:
        box16[6]: box16;
    connections:
        box16[0].gate++ <--> canal <--> box16[1].gate++;
        box16[1].gate++ <--> canal <--> box16[2].gate++;
        box16[1].gate++ <--> canal <--> box16[4].gate++;
        box16[3].gate++ <--> canal <--> box16[4].gate++;
        box16[4].gate++ <--> canal <--> box16[5].gate++;
}

simple box16
{
    parameters:
        @signal[arrival](type="long");
        @statistic[hopCount](title="hopCount"; source="arrival"; record=vector, stats; interpolationmode=none);
        @display("i=block/rxtx");
        
    
    gates:
        inout gate[];
}



