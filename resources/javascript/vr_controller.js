vr_controller = function(entity)
{
    const leftAimName = "leftAim"
    const leftGripName = "leftGrip"
    const rightAimName = "rightAim"
    const rightGripName = "rightGrip"
    const hmdName = "hmd"
    const vrHolderName = "vrHolder"

    let dxy = new Vec2(0, 0)
    let dt = 0

    this.OnCreated = function(){
        this.position = entity.GetLocalTranslation();
        System.Print(this.position)

        System.Input.ControllerEvent(
            System.InputKeyCode.LEFT_THUMBSTICK_X,
            (xValue) => {
                dxy.x = dt * xValue
            }
        )

        System.Input.ControllerEvent(
            System.InputKeyCode.LEFT_THUMBSTICK_Y,
            (yValue) => {
                dxy.y = dt * yValue
            }
        )

        let leftAim = entity.GetScene().GetEntityByName(leftAimName)
        let leftGrip = entity.GetScene().GetEntityByName(leftGripName)
        let rightAim = entity.GetScene().GetEntityByName(rightAimName)
        let rightGrip = entity.GetScene().GetEntityByName(rightGripName)

        System.Input.LeftAimEvent((transform) => {
            leftAim.SetLocalTransform1(transform);
        })

        System.Input.LeftGripEvent((transform) => {
            leftGrip.SetLocalTransform1(transform);
        })

        System.Input.RightAimEvent((transform) => {
            rightAim.SetLocalTransform1(transform);
        })

        System.Input.RightGripEvent((transform) => {
            rightGrip.SetLocalTransform1(transform);
        })
    }

    this.OnUpdated = function(timestep){
        dt = timestep;

        let rotEntity = entity.GetScene().GetEntityByName(hmdName)
        let rotTransform = rotEntity.GetLocalTransform()

        let vx4 = System.Math.Multiply(rotTransform, new Vec4(1, 0, 0, 0))
        let vz4 = System.Math.Multiply(rotTransform, new Vec4(0, 0, 1, 0))

        let vx3 = new Vec3(vx4)
        let up = new Vec3(0, 1, 0)

        let vz3_f = System.Math.Cross(vx3, up)
        vz3_f = System.Math.Normalize(vz3_f)
        let vx3_f = System.Math.Cross(up, vz3_f)
        vx3_f = System.Math.Normalize(vx3_f)

        vx4 = new Vec4(vx3_f)
        vz4 = new Vec4(vz3_f)

        let holder = entity.GetScene().GetEntityByName(vrHolderName)
        let holderTransform = holder.GetLocalTransform()

        holderTransform.c3.z -= vz4.z * dxy.y
        holderTransform.c3.x -= vz4.x * dxy.y

        holderTransform.c3.x += vx4.x * dxy.x
        holderTransform.c3.z += vx4.z * dxy.x

        let follower = entity.GetScene().GetEntityByName(vrHolderName)
        follower.SetLocalTransform1(holderTransform) 
    }

    this.OnDestroyed = function(){

    }
}