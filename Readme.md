# Atmel ATTiny2313 8 channel logic analyzer

This is the hardware component of a very barebones 8 channel logic analyzer 
that pumps out the 8 logic values out via serial. A serial2USB converter is
assumed to be hooked to the SERIAL pins on the circuit diagram.

This circuit / code doesn't actually analyze anything, it simply pumps 
out logic values.

The "analyzer" part of this project will come shortly in the form of a Java
program that visualizes the 8 channels, provides trigger logic etc.

## Configuration
The serial connection is configured at 38400 baud which is the fastest that 
was reliably possible on the internal oscillator 8MHz clock of the ATTiny.

If you extend the circuit and connect a faster external crystal you will 
be able to get higher baudrates.

Other than that there is nothing to configure.

The circuit can be powered over USB, just connect the respective pins from
the serial2USB converter to the VCC/GND pins through the ICSP header.

## A word of caution
Note that the circuit is very barebones, because it was made to explore what
is possible. For a real-life circuit you will want to add measures to protect
agains over current and over voltage, wrong polarization etc. 

Also, a on/off switch and a "on" LED might be useful.

## License
This code is released under the GNU General Public License, version 3. See 
the License.txt file for details.

The UART library in use is based on a library written by Peter Fleury 
<pfleury@gmx.ch> http://jump.to/fleury

His code was published under GNU General Public License, version 2.

The library was further extended by Tim Sharpe to include additional functions 
such as uart_available() and uart_flush(). (See uart.h for details).

His work is released under the GNU General Public License.

Mr Sharpe's additions in turn are based on the Arduino HardwareSerial.h,
released under LGPL, version 2.1.
	
The full text of the license can be found in the ./uart folder and online:
http://opensource.org/licenses/GPL-2.0
