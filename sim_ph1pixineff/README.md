Pixel hit inefficiency simulation, based on the /DataFlow package from Hans-Christian Kaestli

Simulation is done in two steps:

1) read and merge track hits from the geant-based simulation of protons passing through Pixel telescope with 8 ROCs (perpendicular to the beam)
- only a single ROC is chosen from the gean simulation output and hits from multiple events are merged together, based on a randomly generated hitRate 
- output is in the same format as the input used by the /DataFlow package 

code: geantTracks/savehits.c

output: geantTracks/output_geant.root

2) simulate hit inefficiency within the /DataFlow package
- use as input signal hits from geant, as prepared in previous step

input: DataFlow/testbeam.steer