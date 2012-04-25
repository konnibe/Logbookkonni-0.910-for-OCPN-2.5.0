Logbook Konni for OpenCPN
==========================

Building from source
--------------------
Please use Git for download and don't hit the downloads-link above to create a .zip or tar.zip !!
 
Clone the git repository to <OPENCPN_SOURCE_TLD>/plugins/LogbookKonni_pi (the _pi at the end matters!)
---------------------------------------------------------------------------

cd <OPENCPN_SOURCE_TLD>/plugins
open a terminal here and copy/paste this command:
 
git clone git://github.com/konnibe/LogbookKonni.git LogbookKonni_pi

Run this commands:

cd ../build
cmake ..
cmake --build . --target logbookkonni_pi
sudo make install

that's it.

Install Layouts (Linux only)
----------------------------
they are nessesary to display data e.g. in a browser

start opencpn
select in the toolbar Options/Plugins and enable the plugin.
click the peferences-button
in the dialog click "Install" below the label "Install Layouts"
in the filedialog select in the directory '<OPENCPN_SOURCE_TLD>/plugins/LogbookKonni_pi' the file 'OpenCPN_Logbook_Layouts.zip'
