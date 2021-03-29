#include "ros/ros.h"
#include "ball_chaser/DriveToTarget.h"
#include <sensor_msgs/Image.h>
#include <vector>
#include <algorithm>

using std::vector;

// Define a global client that can request services
ros::ServiceClient client;

// This function calls the command_robot service to drive the robot in the specified direction
void drive_robot(float lin_x, float ang_z)
{
    // TODO: Request a service and pass the velocities to it to drive the robot
    ROS_INFO("Drive the robot: velocities - linear: %1.2f, -angular:%1.2f", lin_x, ang_z);

    ball_chaser::DriveToTarget velocity;
    velocity.request.linear_x = lin_x;
    velocity.request.angular_z = ang_z;

    if (!client.call(velocity))
        ROS_ERROR("Failed to call the DriveToTarget service.");
}

// This callback function continuously executes and reads the image data
void process_image_callback(const sensor_msgs::Image img)
{

    int white_pixel = 255;
    int left_counter = 0;
    int forward_counter = 0;
    int right_counter = 0;    

    int margin = img.width/3;

    // TODO: Loop through each pixel in the image and check if there's a bright white one
    for (int i = 0; i < img.height*img.step; i+=3)
    {
        // check if there's a bright white one            
        if ((img.data[i] == white_pixel) && (img.data[i+1] == white_pixel) && (img.data[i+2] == white_pixel))
        {
            int j = (i/3) % img.width;   
            // Find the side of the image
            if(j <= margin) 
            {
                left_counter+=1;
            }
            else if (j > 2*margin) 
            {
                right_counter+=1;
            }
            else {forward_counter+=1;}
        }
    }

    // Identify if this pixel falls in the left, mid, or right side of the image
    // Depending on the white ball position, call the drive_bot function and pass velocities to it
    vector<int> direction_vector {left_counter, forward_counter, right_counter};
    int direction_max = *max_element(direction_vector.begin(), direction_vector.end());    

    // Request a stop when there's no white ball seen by the camera        
    if (direction_max == 0) {drive_robot(0.0,0.0);}
    // Call the drive_bot function and pass velocities to it when see white ball 
    else if (direction_max == left_counter) {drive_robot(0.0,0.8);}
    else if (direction_max == forward_counter) {drive_robot(0.5,0.0);}
    else {drive_robot(0.0,-0.8);}
}

int main(int argc, char** argv)
{
    // Initialize the process_image node and create a handle to it
    ros::init(argc, argv, "process_image");
    ros::NodeHandle n;

    // Define a client service capable of requesting services from command_robot
    client = n.serviceClient<ball_chaser::DriveToTarget>("/ball_chaser/command_robot");

    // Subscribe to /camera/rgb/image_raw topic to read the image data inside the process_image_callback function
    ros::Subscriber sub1 = n.subscribe("/camera/rgb/image_raw", 10, process_image_callback);

    // Handle ROS communication events
    ros::spin();

    return 0;
}