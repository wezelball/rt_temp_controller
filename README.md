rt_temp_controller
==================

Radio Telescope Temperature Controller

This is a homebrew but high-performance temperature controller
for a single-dish radio telescope.

The sketch is now working with 2 selectable channels of simulated
data. The next step is to perform autoscaling and axis label
update for y-axis.

11/25/14
Auto scaling and axis update now works. There is a little bug that
relates to taking absolute readings off of the pot. When switching
between channel 1 and channel 2, there is normally a large jump in
the graph as the routine reads the current pot value and applies
it to the channel.

I need to read the current pot value and assign it to the current
window value, and djust from there.

11/26/14
The jumping display bug is now mostly fixed, and I'm using Geany as an
editor, which is far better. Still working on indentation issues.

There are some other bugs that I want to squash before refactoring.

Also note that the pot should adjust the setpoint as well as moving the
window to center the setpoint in it.

04/12/15
Setup to use a Raspberry Pi model B to develop code for the Arduino.
Upgraded GLCD to OpenGLCD.

04/18/15
I have a functioning live display for channel 1 LNA port, but I need 
to rethink the way the program works, as my current program is not
very useful.

When the switch is flipped to a channel position, the setpoint for that 
channel should be initialized at the current temperature. The pot should
allow adjustment above/below that temperature by a limited amount.  
When the switch is thrown to the center "neutral" position, the window
size around the setpoint should be adjustable. 

If the switch is returned to that channel position, the setpoint should
not change until the pot is moved, and the window size should remain
constant.

If the switch is moved to the other channel, the screen should update
to that channel and adjustments would be the same as above.
