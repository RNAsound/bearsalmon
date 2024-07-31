Bearsalmon (PD external)
====
Created by: Aren Akian

Last updated: Jul 30, 2024


## About
 - Bearsalmon is a Pure Data (PD) external written in C, built on top of the PD “vanilla” library

- Bearsalmon can act as a buffer shuffler/swapper between two pd Array objects (⇧⌘ + A on MacOS or ⇧Ctl + A on Windows/Linux)

- Like a Bear slicing a Salmon with his claw, Bearsalmon lets the user chop, shuffle, and mangle up a sample, wavetable, or any other arbitrary array in PD. 

## Challenges & Future Plans
Writing a PD external itself is quite a steep learning curve at first, but proves to be an incredibly powerful extension to PD’s already-vast capabilities.  
- Designing the external, namely, the decision to interface via messages, and which prototypes to use was the biggest challenge. 
- However, messages proved to be a very extendable interface. Bearsalmon was only originally intended to “cut” through Arrays of samples (by setting regions of the sample to zero). 
- As time went on, I developed the “swap” and “shuffle” message protocols that allow the user to swap regions of samples between two array objects.

In the future I would like to add more message input protocols, and develop a VST/Max4Live version using Max/MSP


## Table of Contents

## Installation
To use Bearsalmon, you need to install the correct compiled binary for your system, in your externals folder. 

    - Mac (any 64 bit architecture): .pd_Darwin

    - Linux (any 64 bit architecture): .pd_linux

    - Windows (any 64 bit architecture): .dll

1. Find the location your PD externals folder
    - Open PD > Preferences > Path to see the default path to your externals folder 

2. Copy the “bearsalmon” folder containing the compiled binaries and the bearsalmon-help.pd files into the externals folder

Typically, the default externals path is as follows:
        
### macOS:

| Scope | Path |
| ------ | ------- |
| Application-specific: |  /$PdPath/Contents/Resources/extra (this is inside the Pd Application (like Pd-0.49-1 in ~/Applications); right click it and choose "Show Package Contents", then navigate to "Resources/extra") |
| User-specific: |  ~/Library/Pd (/Users/user_name/Library/Pd) |
| Global: |  /Library/Pd

### Windows
| Scope | Path |
| ------ | ------- |
| Application-specific: | %ProgramFiles(x86)%\Pd\extra (for 64-bit OS and 32-bit Pd) or %ProgramFiles%\Pd\extra; this is inside the Pd Application (usually in C:\Program Files). This folder needs to be set to writeable.
| User-specific: | %AppData%\Pd (usually in C:\Users\user_name\AppData\Roaming\Pd).
| Global: | %CommonProgramFiles%\Pd (usually in C:\Program Files\Common Files\Pd).

### GNU/Linux

| Scope | Path |
| ------ | ------- |
| Application-specific: |  /usr/lib/pd/extra if installed via a package manager (apt-get) or /usr/local/lib/pd/extra if compiled by yourself.
| User-specific: | ~/.local/lib/pd/extra (preferred since version Pd-0.47-1) or ~/pd-externals (deprecated but still usable).
| Global: | /usr/local/lib/pd-externals.

In most cases, the “User-Specific” path is reccomended.
- If unsure, you can add a search path to your custom install location. 



How to Run
=======
**_Note_**: Bearsalmon requires two sample arrays as its input parameters, otherwise the object cannot be created

1. Open a PD project
2. Inside the patcher, create two sample arrays, “array0” and “array1”
    - (⇧⌘ + A on MacOS or ⇧Ctl + A on Windows/Linux)
3. Create an object (⇧⌘ + 1 or ⇧Ctl + 1) 
    - type in “bearsalmon array0 array1”  
4. Send messages to the bearsalmon object
    - In order to manipulate the two arrays, you must send messages to the bearsalmon object via a message box or a Bang object, for example “cut 250” 
    - The message prototypes are detailed below

How to Use
=======
1. Follow the steps above to create two sample _arrays_, and a _bearsalmon_ object that contains them. Use a _tabread~_ to play either of the arrays (like in the help file)
2. Load some samples into both arrays using _tabwrite_ or _read_
    - These can be wavetables, oscillator shapes, or audio samples of any length
3. Then send messages to _bearsalmon_ such as “_cut 0 150 300_” or “_swap 128 256_” or “_shuffle 32_”
    - Make sure the bounds of your messages fit within the sample length of the two arrays 
5. Get creative and set off several messages at once using _bangs_, or sequences of messages using metronomes or other automation! 
    - Mangle those samples like the bear you are. 
6. For examples or further help, open “_bearsalmon_help.pd_” included in this repo.

Inputs & Message Prototypes
===

### Bang
**Info:** Sending a bang to the bearsalmon object will output some information regarding the arrays.

**Arguments:** 
- _BANG_ - output some information


***
### Cut
**Message box prototype:** “_cut array_sel startframe endframe_”

**Arguments:**
- _array_sel_ - The target array. Can be either 0 or 1, corresponding to the _array0_ and _array1_, respectively
- _startframe_ – the first frame in the range of array elements to be cut
- _endframe_ – the last frame in the range of array elements to be cut

**Info:** Cut modifies the target array over the range startframe, to endframe, setting the values over this range to zero.
- This method was implemented such that each call to “cut” also updates the GUI of the array in the patch window, providing useful (and fun) visual feedback to the user.



***
### Swap
**Message box prototype:** “_swap startframe endframe_”

**Arguments:**
- _startframe_ – the first frame in the range of elements to be swapped
- _endframe_ – the last frame in the range of elements to be swapped

**Info:** This method is similar to cut, as it operates on a given range of elements. However, swap modifies both arrays in the bearsalmon object by swapping their elements over the specified range. The C-implementation of this uses the classic method of array swapping: 
Given arrayA and arrayB, and a range of indices:
1. Store the desired range of elements from arrayB in a temporary local buffer
2. Copy the desired range from arrayA to the correct indices of arrayB
3. Copy the data from the temporary buffer to the correct indices of arrayA

***
### Shuffle

**Message box prototype:** “_shuffle segment_width_”

**Arguments:**
- _Segment_width_ – the desired width of the segments to be shuffled, in frames


**Info:** _shuffle_ modifies both arrays so that they contain alternating segments of samples from both arrays, of length _segment_width_. Basically, this function performs a _swap_, on every other segment of elements. 

If given a _segment_width_, _n_, the first array will contain _n_ samples from the second, then _n_ samples of its original array, then _n_ samples from the second, and so on. 

For Example:
- _Array0_ = {0,0,0,0, 0,0,0,0}
- _Array1_ = {1,1,1,1, 1,1,1,1}

Sending the bearsalmon object the message: “_swap 2_” makes it so that :
- _Array0_ = {1,1,0,0, 1,1,0,0}
- _Array1_ = {0,0,1,1, 0,0,1,1}

The C implementation of _shuffle_ uses calls to a modified version of the _swap_ function I wrote, except it does not update the GUI in between each swap. This “quickswap” function helps improve the speed of this shuffle operation.

Troubleshooting
===
If the external does not run in PD, or causes any issues, you may need to _recompile_ the included source code for your system. I recommend using [pd-lib-builder](https://github.com/pure-data/pd-lib-builder)

Credits
===
- [PD Docs: Externals](https://msp.ucsd.edu/Pd_documentation/x4.htm)
- [pd-lib-builder](https://github.com/pure-data/pd-lib-builder)
- _Designing Audio Objects for Max/MSP and Pd_ by Eric Lyon
