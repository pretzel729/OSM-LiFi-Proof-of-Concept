# OSM-LiFi-Proof-of-Concept
Proof of concept for reading arbitrary data into the OSM 2.1 microlights

Upload OSMLiFiOutput.ino to a OSM 2.1 microlight.
Upload OSMLiFiInput.ino to a different OSM 2.1 microlight.
Optional: Open the serial monitor for the OSM running the input sketch.
Power both microlights and face their LEDs to each other to within some inches of eachother and wait.
After a default wait time of 10 seconds (10 bytes @ 8 bits/second), the input OSM should now repeat the data played by the output OSM.
