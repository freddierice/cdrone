package base

import (
	"../proto"
)

// setMode tells the drone to switch modes.
func (b *Base) setMode(mode proto.UpdateMode) {
	b.messageChan <- &proto.Update{
		Mode: mode,
	}
}

// setCommand gives the drone a command
func (b *Base) setCommand(command proto.UpdateCommand) {
	b.messageChan <- &proto.Update{
		Cmd: command,
	}
}

// SetVelocity sets the velocity for the drone.
func (b *Base) SetVelocity(roll, pitch, yaw, throttle uint32) {
	b.messageChan <- &proto.Update{
		Velocity: &proto.UpdateVelocity{
			Roll:     roll,
			Pitch:    pitch,
			Yaw:      yaw,
			Throttle: throttle,
		},
	}

}

// Arm sends an arm command to the drone.
func (b *Base) Arm() {
	b.setMode(proto.UpdateMode_ARM)
}

// Disarm sends a disarm command to the drone.
func (b *Base) Disarm() {
	b.setMode(proto.UpdateMode_DISARM)
}

// Takeoff sends a takeoff command to the drone.
func (b *Base) Takeoff() {
	b.setMode(proto.UpdateMode_TAKEOFF)
}

// Position tells the drone to switch to position mode.
func (b *Base) Position() {
	b.setMode(proto.UpdateMode_POSITION)
}

// ResetPosition tells the drone to reset the position
func (b *Base) ResetPosition() {
	b.setCommand(proto.UpdateCommand_RESET_POSITION)
}
