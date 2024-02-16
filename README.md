# PWM-Servo-Control-Project

### **Project Requirements:**  
Create a project that allows users to control servo motors using the PWM Click add-on board with a Raspberry Pi.
PWM Click should interface with the Raspberry Pi using the I2C communication protocol.
The project should provide a user-friendly interface (a terminal accepting keyboard keys) for controlling servo motors connected to the Raspberry Pi via the PCA9685 PWM controller.
Additionally, the project should include functionality for recording servo movements allowing for future analysis.

### **Modules:**
* Raspberry Pi - Raspberry Pi 1 B+ was used for testing.
* PCA9685 PWM Click controller - the datasheet for the module : https://download.mikroe.com/documents/datasheets/PCA9685_datasheet.pdf
* Two HS-422 Deluxe Standard Servo motors using 5V as Voltage
  
### **Implementation:**

* **PWM Click Driver**  
 The pwm_click driver is the core of the project its designed to facilitate the interaction between a Raspberry Pi and the PWM Click add-on board using the I2C communication protocol. The pwm_click_ioctl is a     function implemented in the driver to handle input/output control (IOCTL) requests from userspace (communicate with the driver from the application).

* **The Application**  
 The application is designed to allow the control of two seperate servo motors, enabling users to control servo movement via key inputs or specific angle values.
 The functionality of recording and saving servo movements in a CSV file for future analysis is included in the application.

### **Usage:**  
The provided Makefile is meant to be used directly on the ARM system. In orded to compile/insmod the driver successfully you will have to ensure that your ARM-based system has the necessary development tools installed. On the other hand if you wish to cross-compile the driver you'll need a cross-compilation toolchain that targets ARM architecture.

### **Summary:**  
This project involves the development of a PWM (Pulse Width Modulation) servo motor control system using the PCA9685 PWM controller. 
The system is designed to allow the control of two seperate servo motors, enabling users to control servo movement via key inputs or specific angle values.
The project includes the functionality of recording and saving servo movements in a CSV file for future analysis.


