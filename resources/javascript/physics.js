System.Physics = {}

System.Physics.GeometryType =
{
    SPHERE: 0,
    // PLANE: 1, // Not supported yet
    CAPSULE: 2,
    BOX: 3
}

//FIXME: C++ segfaults if JS keys are modified
System.Physics.BoxGeometry = function(x, y, z)
{
    this.GeometryType = System.Physics.GeometryType.BOX;
    this.halfExtent = Vec3(
        x? x: 0.5,
        y? y: 0.5,
        z? z: 0.5
    );
}

System.Physics.SphereGeometry = function(radius)
{
    this.GeometryType = System.Physics.GeometryType.SPHERE;
    this.radius = radius? radius: 0.5;
}

System.Physics.CapsuleGeometry = function(halfHeight, radius)
{
    this.GeometryType = System.Physics.GeometryType.CAPSULE;
    this.halfHeight = halfHeight? halfHeight: 0.5;
    this.radius = radius? radius: 1;
}