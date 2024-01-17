#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdbool.h>

#define PWM_DEVICE_PATH "/dev/pwm_click_device"  // Device file path

//Defining IOCTL commands
#define PWM_SET_FREQUENCY_CHANNEL _IOW('p', 1, unsigned int)
#define PWM_SET_DUTY_CYCLE_CHANNEL1 _IOW('p', 2, unsigned int)
#define PWM_SET_DUTY_CYCLE_CHANNEL2 _IOW('p', 3, unsigned int)


//Functions used for converting the angle value of the servo motor to its duty cycle and vice versa 
int angle_to_duty_cycle(float angle) {
   
    float min_angle = 0.0;
    float max_angle = 180.0;
    float min_duty_cycle = 2.0;
    float max_duty_cycle = 14.0;


    return (int)(min_duty_cycle + (angle - min_angle) / (max_angle - min_angle) * (max_duty_cycle - min_duty_cycle));
}

int duty_cycle_to_angle(int duty_cycle) {
    float min_angle = 0.0;
    float max_angle = 180.0;
    float min_duty_cycle = 2.0;
    float max_duty_cycle = 14.0;

    
    if (duty_cycle < min_duty_cycle || duty_cycle > max_duty_cycle) {
      
        return -1.0;  //Value indicates an error
    }

    return (int)(min_angle + (duty_cycle - min_duty_cycle) / (max_duty_cycle - min_duty_cycle) * (max_angle - min_angle));
}



//Current duty cycle values of servo as static integers.

static int servo1_duty_cycle=2;
static int servo2_duty_cycle=2;

int main() {
	
	//Open the Record servo movements file and write out the header of the csv file
	FILE *csvFile = fopen("record_servo_movements.csv", "w");
	
	   if (csvFile == NULL) {
        fprintf(stderr, "Error opening file for writing.\n");
        return 1;
    }
	fprintf(csvFile,"Step,Servo1,Servo2\n");
	
	
    int pwm_fd; // used to open the pwm device file
	int num=1; // used to keep track of number of servo movements, for the purpose of recording the movements in the csv file.
	
    // Open the PWM device file
    pwm_fd = open(PWM_DEVICE_PATH, O_RDWR);
    if (pwm_fd < 0) {
        perror("Error opening PWM device");
        exit(EXIT_FAILURE);
    }

    // Set PWM frequency 
    unsigned int frequency = 50;  // 50 Hz for standard servos
    if (ioctl(pwm_fd, PWM_SET_FREQUENCY_CHANNEL, frequency) < 0) {
        perror("Error setting PWM frequency");
        close(pwm_fd);
        exit(EXIT_FAILURE);
    }

		
    // Move both servo motors to 0 degrees(starting position) with an IOCTL call that sends data to kernel space
    unsigned int duty_cycle_0_degrees = 2;  
    if (ioctl(pwm_fd, PWM_SET_DUTY_CYCLE_CHANNEL1, duty_cycle_0_degrees) < 0) {
        perror("Error setting PWM duty cycle");
        close(pwm_fd);
        exit(EXIT_FAILURE);
    }

	if (ioctl(pwm_fd, PWM_SET_DUTY_CYCLE_CHANNEL2, duty_cycle_0_degrees) < 0) {
        perror("Error setting PWM duty cycle");
        close(pwm_fd);
        exit(EXIT_FAILURE);
    }
	
	fprintf(csvFile, "%d, %d, %d\n", num, 0, 0);
	
    // Wait for the servo to reach 0 degrees (2 second wait)
    usleep(2000000); 
	
 
	printf("Input method : Moving Tabs(1)/Exact angle values(2) \n");
	char choice;
	scanf(" %c",&choice);
	if(choice=='1')
	{
		char ch='0';
		
		while(ch!='#')
		{
			bool first=false; bool second=false;
			printf("Up(U) / Down(D) / Left(L) / Right(R) / End(#) \n");
			scanf(" %c",&ch);
			
			//char charClear; while ((charClear = getchar()) != '\n' && charClear != EOF);   //clear out the buffer
			
			/*
			The tabs Up(U) and Down(D) are used to increase/decrease the value of the duty cycle of the second servo motor with the maximum/minimum value of the 
			duty cycle being 14/2 (corresponding to 180`/0` angle values of the servo), the increment/decrement value is 1 (corresponding to 15`) and 12 instances of any tab are
			required to get the full range of the servo (e.g 12 U tabs to go from 0` to 180` for the second servo).
			The Right(R) and Left(L) tabs provide equivalent functionality for the first servo.
			'#' is used to indicate the end of input.
			*/
			if(ch=='U' || ch=='u')
			{
				if(servo2_duty_cycle<14)
				{
				servo2_duty_cycle++; second=true; num++;
				}
			}
			if(ch=='D' || ch=='d')
			{
				if(servo2_duty_cycle>2)
				{
				servo2_duty_cycle--;second=true; num++;
				}
			}
			if(ch=='R' || ch=='r')
			{
				if(servo1_duty_cycle<14)
				{
				servo1_duty_cycle++; first=true; num++;
				}
			}
			if(ch=='L' || ch=='l')
			{
				if(servo1_duty_cycle>2)
				{
				servo1_duty_cycle--;first=true; num++;
				}
			}
			
			
			if(second)
			{
				   if (ioctl(pwm_fd, PWM_SET_DUTY_CYCLE_CHANNEL2, servo2_duty_cycle) < 0) {
					perror("Error setting PWM duty cycle");
					close(pwm_fd);
					exit(EXIT_FAILURE);
						}
						
					fprintf(csvFile, "%d, %d, %d\n", num, duty_cycle_to_angle(servo1_duty_cycle), duty_cycle_to_angle(servo2_duty_cycle));	
						
			}
			if(first)
			{
				 if (ioctl(pwm_fd, PWM_SET_DUTY_CYCLE_CHANNEL1, servo1_duty_cycle) < 0) {
					perror("Error setting PWM duty cycle");
					close(pwm_fd);
					exit(EXIT_FAILURE);
						}
						
					fprintf(csvFile, "%d, %d, %d\n", num, duty_cycle_to_angle(servo1_duty_cycle), duty_cycle_to_angle(servo2_duty_cycle));	
			}
		}
		
	}
	else if(choice=='2')
	{
	    printf("Input angle values for both servo (0`- 180`) with accuracy of 15` or # for end(for both)\n");
	
		/*
		This option allows the user to manually input angle values for both servo motors. The user is prompted to
		enter values between 0 and 180 degrees with an accuracy of 15 degrees.
		e.g 0`-15` input is 0`, 15`-30` input is 15` etc.
		'#''#' is used to indicate the end of input.
		*/
			
		char str1[] = "12345"; char str2[] = "12345";
		int num1,num2;
		
		while(str1[0]!='#' || str2[0]!='#')
		{
			char *errPtr1; char *errPtr2;// Used to check for conversion errors
			printf("Value for First Servo\n"); scanf("%s",str1);
			printf("Value for Second Servo\n"); scanf("%s",str2);
			// Using strtol to convert string to long.
			if(str1[0]!='#' || str2[0]!='#')
			{
				long num1Check = strtol(str1, &errPtr1, 10);
				long num2Check = strtol(str2, &errPtr2, 10);
				// Check for conversion errors
				if ((*errPtr1 != '\0') || (*errPtr2 != '\0')) {
				printf("Error: Invalid input\n");
				}
				else
				{
					if(num1Check>180 || num1Check<0 || num2Check>180 || num2Check<0)
						printf("Invalid values\n");
					else
					{
						num1=(int)num1Check;
						num2=(int)num2Check;
						int num1Duty=angle_to_duty_cycle(num1);
						int num2Duty=angle_to_duty_cycle(num2);
						
						if((servo1_duty_cycle!=num1Duty) || (servo2_duty_cycle!=num2Duty))
						{
							servo1_duty_cycle=num1Duty; servo2_duty_cycle=num2Duty; num++;
							if (ioctl(pwm_fd, PWM_SET_DUTY_CYCLE_CHANNEL1,num1Duty) < 0) {
							perror("Error setting PWM duty cycle");
							close(pwm_fd);
							exit(EXIT_FAILURE);
							}
							
							if (ioctl(pwm_fd, PWM_SET_DUTY_CYCLE_CHANNEL2,num2Duty) < 0) {
							perror("Error setting PWM duty cycle");
							close(pwm_fd);
							exit(EXIT_FAILURE);
							}
							
							fprintf(csvFile, "%d, %d, %d\n", num, duty_cycle_to_angle(num1Duty), duty_cycle_to_angle(num2Duty));	
						}
					}
				}
			}
		}
	}
	else printf("Invalid input method");
	
    // Close the opened files
    close(pwm_fd);
	fclose(csvFile);
	
    return 0;
}