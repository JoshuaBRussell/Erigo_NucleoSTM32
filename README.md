# Erigo_NucleoSTM32
Repo for code written for a STM32-Nucleo board intended to collect data from an Erigo.


# Needed Software
* [STM32CubeMX](https://www.st.com/en/development-tools/stm32cubemx.html) - This is used to initialize the uC configuration.
* [SW4STM32](https://www.st.com/en/development-tools/sw4stm32.html) - An IDE based off of Eclipse. 
* There are some misc. drivers needed to communicate with the STM32, but those are made obvious throughout the install process. 

# Importing STM32CubeMX project *into* System Workbench

Once a project has been made using **CubeMX** in can be imported into **System Workbench** by following the instructions [here](http://www.openstm32.org/Importing%2Ba%2BSTCubeMX%2Bgenerated%2Bproject). 

*Note: It mentions at the bottom of the guide that more needed to be done in order to get the debug feature working. That wasn't the case for me.* 

# IDE Usage

Once the project is imported you can press the *Build* button (looks like a hammer) to build the project and make sure everything is okay. 

Make sure to place any *"user"* code into the noted comment blocks. This is allows **CubeMX** (and therefore us) to change the configuration of the uC without **CubeMX** deleting our code. 
