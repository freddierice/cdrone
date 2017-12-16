package base

import (
	"../proto"
)

// setMode tells the drone to switch modes.
func (b *Base) setMode(mode proto.Mode) {
	b.messageChan <- &proto.Update{
		Mode: mode,
	}
}

// setCommand gives the drone a command
func (b *Base) setCommand(command proto.Command) {
	b.messageChan <- &proto.Update{
		Command: command,
	}
}

// SetPosition sets the position for the drone.
func (b *Base) SetPosition(x, y, z float64) {
	b.messageChan <- &proto.Update{
		Position: &proto.Position{
			X: x,
			Y: y,
			Z: z,
		},
	}
}

// SetVelocity sets the velocity for the drone.
func (b *Base) SetVelocity(x, y, z float64) {
	b.messageChan <- &proto.Update{
		Velocity: &proto.Velocity{
			X: x,
			Y: y,
			Z: z,
		},
	}
}

// Arm sends an arm command to the drone.
func (b *Base) Arm() {
	b.setMode(proto.Mode_ARM)
}

// Disarm sends a disarm command to the drone.
func (b *Base) Disarm() {
	b.setMode(proto.Mode_DISARM)
}

// Takeoff sends a takeoff command to the drone.
func (b *Base) Takeoff() {
	b.setMode(proto.Mode_TAKEOFF)
}

// Touchdown sends a takeoff command to the drone.
func (b *Base) Touchdown() {
	b.setMode(proto.Mode_TOUCHDOWN)
}

// Position tells the drone to switch to position mode.
func (b *Base) Position() {
	b.setMode(proto.Mode_POSITION)
}

// Velocity tells the drone to switch to velocity mode.
func (b *Base) Velocity() {
	b.setMode(proto.Mode_VELOCITY)
}

// ResetPosition tells the drone to reset the position
func (b *Base) ResetPosition() {
	b.setCommand(proto.Command_RESET_POSITION)
}
