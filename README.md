# OSM-LiFi-Proof-of-Concept
Proof of concept for reading arbitrary data into the OSM 2.1 microlights

Works at 8 bits/second or about 3 minutes/1 Vectr mode.
The sketches would have to be tweaked for higher transfer rates to work.
More info in the comments of OSMLiFiInput.ino

Upload OSMLiFiOutput.ino to a OSM 2.1 microlight.
Upload OSMLiFiInput.ino to a different OSM 2.1 microlight.
Optional: Open the serial monitor for the OSM running the input sketch.
Power both microlights and face their LEDs to each other to within some inches of eachother and wait.
After a default wait time of 10 seconds (10 bytes @ 8 bits/second), the input OSM should now repeat the data played by the output OSM.

If you use these for anything let me know so I can link it here incase other people Google for OSM chip communication.
