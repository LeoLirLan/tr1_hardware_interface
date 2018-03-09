#include <sstream>
#include <tr1_hardware_interface/tr1_hardware_interface.h>
#include <tr1cpp/tr1.h>
#include <tr1cpp/arm.h>
#include <tr1cpp/joint.h>

namespace tr1_hardware_interface
{
	TR1HardwareInterface::TR1HardwareInterface(ros::NodeHandle& nh) \
		: nh_(nh)
	{
		// Initialize shared memory and interfaces
		init();

		// Create the controller manager
		controller_manager_.reset(new controller_manager::ControllerManager(this, nh_));

		// Get period and create timer
		nh_.param("/tr1/hardware_interface/loop_hz", loop_hz_, 0.1);
		ROS_DEBUG_STREAM_NAMED("constructor","Using loop freqency of " << loop_hz_ << " hz");
		ros::Duration update_freq = ros::Duration(1.0/loop_hz_);
		non_realtime_loop_ = nh_.createTimer(update_freq, &TR1HardwareInterface::update, this);

		this->tr1 = tr1cpp::TR1();

		ROS_INFO_NAMED("hardware_interface", "Loaded generic_hardware_interface.");
	}

	TR1HardwareInterface::~TR1HardwareInterface()
	{
	}

	void TR1HardwareInterface::init()
	{

		joint_mode_ = 3; // ONLY EFFORT FOR NOW
		// Get joint names
		nh_.getParam("/tr1/hardware_interface/joints", joint_names_);
		if (joint_names_.size() == 0)
		{
		  ROS_FATAL_STREAM_NAMED("init","Not joints found on parameter server for controller, did you load the proper yaml file?");
		}
		num_joints_ = joint_names_.size();

		// Resize vectors
		joint_position_.resize(num_joints_);
		joint_velocity_.resize(num_joints_);
		joint_effort_.resize(num_joints_);
		joint_position_command_.resize(num_joints_);
		joint_velocity_command_.resize(num_joints_);
		joint_effort_command_.resize(num_joints_);

		// Initialize controller
		for (int i = 0; i < num_joints_; ++i)
		{
		  ROS_DEBUG_STREAM_NAMED("constructor","Loading joint name: " << joint_names_[i]);

		  // Create joint state interface
		  joint_state_interface_.registerHandle(hardware_interface::JointStateHandle(joint_names_[i], &joint_position_[i], &joint_velocity_[i], &joint_effort_[i]));

		  // Create position joint interface
		  //position_joint_interface_.registerHandle(hardware_interface::JointHandle(joint_state_interface_.getHandle(joint_names_[i]),&joint_position_command_[i]));

		  // Create velocity joint interface
		  //velocity_joint_interface_.registerHandle(hardware_interface::JointHandle(joint_state_interface_.getHandle(joint_names_[i]),&joint_velocity_command_[i]));

		  // Create effort joint interface
		  effort_joint_interface_.registerHandle(hardware_interface::JointHandle(joint_state_interface_.getHandle(joint_names_[i]),&joint_effort_command_[i]));

		}
		registerInterface(&joint_state_interface_); // From RobotHW base class.
		registerInterface(&position_joint_interface_); // From RobotHW base class.
		registerInterface(&velocity_joint_interface_); // From RobotHW base class.
		registerInterface(&effort_joint_interface_); // From RobotHW base class.
	}

	void TR1HardwareInterface::update(const ros::TimerEvent& e)
	{
		elapsed_time_ = ros::Duration(e.current_real - e.last_real);

		read();
		controller_manager_->update(ros::Time::now(), elapsed_time_);
		write(elapsed_time_);

		//double command = effort_joint_interface_.getHandle("arm1_to_arm2").getCommand();
		//double& command = joint_effort_command_[0];
		//ROS_INFO("JEC: [%f]", command);
	}

	void TR1HardwareInterface::read()
	{
		// Read the joint states from your hardware here
	}

	void TR1HardwareInterface::write(ros::Duration elapsed_time)
	{
		// Send commands in different modes
		for (int i = 0; i < num_joints_; i++)
		{
			tr1.armRight.joints[i].step(joint_effort_command_[i]);
		}

		/*// Move all the states to the commanded set points slowly
		for (std::size_t i = 0; i < num_joints_; ++i)
		{
		  switch (joint_mode_)
		  {
		    case 1: //hardware_interface::MODE_POSITION:
		      // Position
		      p_error_ = joint_position_command_[i] - joint_position_[i];
		      // scale the rate it takes to achieve position by a factor that is invariant to the feedback loop
		      joint_position_[i] += p_error_ * POSITION_STEP_FACTOR / loop_hz_;
		      break;

		    case 2: //hardware_interface::MODE_VELOCITY:
		      // Position
		      joint_position_[i] += joint_velocity_[i] * elapsed_time.toSec();

		      // Velocity
		      v_error_ = joint_velocity_command_[i] - joint_velocity_[i];
		      // scale the rate it takes to achieve velocity by a factor that is invariant to the feedback loop
		      joint_velocity_[i] += v_error_ * VELOCITY_STEP_FACTOR / loop_hz_;
		      break;

		    case 3: //hardware_interface::MODE_EFFORT:
		      ROS_INFO("Effort write: [%f]", joint_effort_command_[i]);
		      break;
		  }
		}*/
	}
}