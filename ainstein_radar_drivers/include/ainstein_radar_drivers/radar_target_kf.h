#ifndef RADAR_TARGET_KF_H_
#define RADAR_TARGET_KF_H_

#include <ros/ros.h>
#include <Eigen/Eigen>

#include <ainstein_radar_msgs/RadarTarget.h>

#define Q_SPEED 1.0
#define Q_AZIM 30.0
#define Q_ELEV 30.0

#define R_RANGE 0.5
#define R_SPEED 1.0
#define R_AZIM 5.0
#define R_ELEV 5.0

#define INIT_VAR_RANGE 1.0
#define INIT_VAR_SPEED 2.0
#define INIT_VAR_AZIM 10.0
#define INIT_VAR_ELEV 10.0

namespace ainstein_radar_drivers
{
  class RadarTargetKF
  {
      class FilterState
      {
      public:
	FilterState( const ainstein_radar_msgs::RadarTarget& target )
	  {
	    range = target.range;
	    speed = target.speed;
	    azimuth = target.azimuth;
	    elevation = target.elevation;

	    cov = ( Eigen::Vector4d() <<
		    std::pow( INIT_VAR_RANGE, 2.0 ),
		    std::pow( INIT_VAR_SPEED, 2.0 ),
		    std::pow( INIT_VAR_AZIM, 2.0 ),
		    std::pow( INIT_VAR_ELEV, 2.0 ) ).finished().asDiagonal();
	  }
	FilterState( const FilterState& state )
	  {
	    range = state.range;
	    speed = state.speed;
	    azimuth = state.azimuth;
	    elevation = state.elevation;

	    cov = state.cov;
	  }
	~FilterState() {}

	friend std::ostream& operator<< ( std::ostream& out, const FilterState& state )
	{
	  out << "Range: " << state.range << std::endl
	      << "Speed: " << state.speed << std::endl
	      << "Azimuth: " << state.azimuth << std::endl
	      << "Elevation: " << state.elevation << std::endl
	      << "Covariance: " << std::endl << state.cov << std::endl;
	}
	
	double range;
	double speed;
	double azimuth;
	double elevation;

	Eigen::Matrix4d cov;

	Eigen::Vector4d asVec( void ) const
	  {
	    return Eigen::Vector4d( this->range,
				    this->speed,
				    this->azimuth,
				    this->elevation );
	  }
	void fromVec( const Eigen::Vector4d& state_vec )
	  {
	    this->range = state_vec( 0 );
	    this->speed = state_vec( 1 );
	    this->azimuth = state_vec( 2 );
	    this->elevation = state_vec( 3 );
	  }

	ainstein_radar_msgs::RadarTarget asMsg( void ) const
	  {
	    ainstein_radar_msgs::RadarTarget t;
	    t.range = range;
	    t.speed = speed;
	    t.azimuth = azimuth;
	    t.elevation = elevation;

	    return t;
	  }
      };

  public:
      RadarTargetKF( const ainstein_radar_msgs::RadarTarget& target );
      ~RadarTargetKF() {}

      friend std::ostream& operator<< ( std::ostream& out, const RadarTargetKF& kf )
	{
	  out << "Time Since Start: " << kf.getTimeSinceStart() << std::endl
	      << "Time Since Update: " << kf.getTimeSinceUpdate() << std::endl;
	}
	
      void process( double dt );
      void update( const ainstein_radar_msgs::RadarTarget& target );

      FilterState getState( void ) const
      {
	return state_post_;
      }
      Eigen::Vector4d computePredMeas( const FilterState& state )
	{
	  return H_ * state.asVec();
	}
      Eigen::Matrix4d computeMeasCov( const FilterState& state )
	{
	  return H_ * state.cov * H_.transpose() + R_;
	}

      double getTimeSinceStart( void ) const
      {
	return ( ros::Time::now() - time_first_update_ ).toSec();
      }

      double getTimeSinceUpdate( void ) const 
      {
	return ( ros::Time::now() - time_last_update_ ).toSec();
      }
      
  private:
      FilterState state_pre_;
      FilterState state_post_;
      
      ros::Time time_first_update_;
      ros::Time time_last_update_;
      
      Eigen::Matrix4d K_;

      // Shared among all filters:
      static Eigen::Matrix4d F_;
      static Eigen::Matrix<double, 4, 3> L_;
      static Eigen::Matrix4d H_;

      static Eigen::Matrix3d Q_;
      static Eigen::Matrix4d R_;
  };

} // namespace ainstein_radar_drivers

#endif // RADAR_TARGET_KF_H_
