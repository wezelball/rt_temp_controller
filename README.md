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
