#include "PlatformDestination.h"
#include <cmath>
#include <cassert>
namespace hg {
    namespace {
        int abs(int a) {
            return a < 0 ? -a : a;
        }
        int sign(int a) {
            assert(a);
            return a / abs(a);
        }
        unsigned pow(unsigned num, unsigned n) {
            unsigned retv(1);
            for(unsigned i(0); i < n; ++i) {
                retv *= num;
            }
            return retv;
        }
        //Returns the integer square-root of num.
        //That is: the greatest integer that is 
        //less than or equal to the squareroot of num
        unsigned isqrt(unsigned num) {
            unsigned guess(1);
            while (guess * guess <= num) {
                ++guess;
            }
            return guess - 1;
        }
        //Returns the integer Nth-root of num.
        //That is: the greatest integer that is 
        //less than or equal to the Nth-root of num
        unsigned inthrt(unsigned num, unsigned n) {
            unsigned guess(1);
            while (pow(guess, n) <= num) {
                ++guess;
            }
            return guess - 1;
        }
        
        int square(int num) {
            return num * num;
        }
    }

    PlatformDestinationComponent::PlatformDestinationComponent(
        int desiredPosition,
        int maxSpeed,
        int acceleration,
        int deceleration) :
        desiredPosition_(desiredPosition),
        maxSpeed_(maxSpeed),
        acceleration_(acceleration),
        deceleration_(deceleration)
    {
    }
    PositionAndVelocity PlatformDestinationComponent::solvePDEquation(
        PositionAndVelocity pnv) const
    {
        if (desiredPosition_ != pnv.position)
		{
			if (abs(desiredPosition_ - pnv.position) <= abs(pnv.velocity) && abs(pnv.velocity) <= deceleration_)
			{
				pnv.velocity = desiredPosition_ - pnv.position;
			}
			else
			{
				int direction(abs(pnv.position - desiredPosition_)/(pnv.position - desiredPosition_));

				if (pnv.velocity * direction > 0)
				{
					pnv.velocity -= direction * deceleration_;
					if (pnv.velocity * direction < 0)
					{
						pnv.velocity = 0;
					}
				}
				else
				{
					// if the platform can still stop if it fully accelerates
					if (abs(pnv.position - desiredPosition_)
							>
                        (square(pnv.velocity - direction * acceleration_)
                         * 3 / (2 * deceleration_)))
					{
						// fully accelerate
						pnv.velocity -= direction*acceleration_;
					}
					// if the platform can stop if it doesn't accelerate
					else if (abs(pnv.position - desiredPosition_) > (square(pnv.velocity)*3/(2*deceleration_)))
					{
						// set speed to required speed
						pnv.velocity = -direction*isqrt(abs(pnv.position - desiredPosition_)*deceleration_*2/3);
					}
					else
					{
						pnv.velocity += direction*deceleration_;
					}
				}
			}
		}
		else
		{
			if (abs(pnv.velocity) <= deceleration_)
			{
				pnv.velocity = 0;
			}
			else
			{
				pnv.velocity += (abs(pnv.velocity)/pnv.velocity)*deceleration_;
			}
		}

		if (abs(pnv.velocity) > maxSpeed_)
		{
			pnv.velocity = sign(pnv.velocity)*maxSpeed_;
		}

		pnv.position += pnv.velocity;
        return pnv;
    }
}//namespace hg
