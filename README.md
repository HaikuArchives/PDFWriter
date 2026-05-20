# PDF Writer
PDF Writer is now part of the print_kit of the [Haiku Project](http://www.haiku-os.org').


## Overview

PDF Writer plugs into Haiku's print_server as a printer driver. This allows  any application that can print to save its output as a PDF.


## Authors

 | Name              | Email                          | Native language |
 | ----------------- | ------------------------------ | --------------- |
 | Philippe Houdoin  | philippe.houdoin@free.fr       | French |
 | Michael Pfeiffer  | michael.pfeiffer@utanet.at     | German |
 | Simon Gauvin      | gauvins@cs.dal.ca              | French / English |


## Installation

1. Expand the distribution archive somewhere
2. Open the newly created folder
3. Run the `Install` script to install the driver


These next steps are only needed if you do not already have a PDF Writer
spooler configured.

4. Open the **Printers** preference panel
5. Click **Add**
6. Choose **Local printer**
   *(Network printing is untested, feedback welcome!)*
7. Type a name for your new PDF Writer spooler
8. Select **PDF Writer** as the kind
9. Select the port to output the PDF
   *(Print to file to generate PDF files)*
10. Click **Add**
11. Click **OK**
12. Enjoy!


## Changes

Read the [`CHANGES`](CHANGES) file for the most recent changes.


## Known Limitations and Bugs

- Limited alpha transparency support only (bitmap/pattern only)
- Not all drawing modes supported
- No security settings support.
- Clipping to inverse BPicture not supported


## Note

BePDF until version 0.9.4 does not take the alpha channel of transparent bitmaps or patterns into account; Acrobat Reader shows them properly!
