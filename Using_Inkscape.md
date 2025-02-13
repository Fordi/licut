# Using Inkscape with licut

Updated 28 Jan 2011

licut was designed around taking vector graphics created with Inkscape
and sending them to a Cricut cutting device. All you need is the Cricut device
and free software - you don't need any cartridges other than what comes with
the device.

There are a number of resources available to help you get started with
Inkscape. It is an excellent vector graphics program similar in features
to Adobe Illustrator and is available for free on all popular platforms;
Linux, Mac OS, and Windows.

There are a couple of guidelines to follow when preparing svg documents
for use with licut:

1. ## Convert text to path

    Once you've finalized your text, highlight your text object and select
    `Path -> Object to path`

    You must do this before licut will send the text outline to the cutter.
    Once you convert your text object to a path, you will not be able to
    edit as text, so you probably want to keep an original version and a
    version with the text objects converted to paths.

    What's a path? A path is a collection of straight and curved line points,
    which is used to direct the cutting blade on the device.

2. ## Unify any overlapping paths

    If you have paths that overlap each other (the overlaps may not show
    if the paths are filled), you want to join them together, otherwise
    the cutter will make more cuts than you may want. To join overlapping
    paths into a single path, select all of the overlapping objects and select
    `Path -> Union`
