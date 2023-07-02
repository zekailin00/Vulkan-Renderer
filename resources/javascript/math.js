System.Math.Type =
{
    Vec2: 1,
    Vec3: 2,
    Vec4: 3,
    Mat3: 4,
    Mat4: 5
}

Vec2 = function(x, y)
{
    this.type = System.Math.Type.Vec2
    this.x = x? x:0;
    this.y = y? y:0;

    this.Print = function()
    {
        System.Print("[", this.x, this.y, "]");
    }
}

Vec3 = function(x, y, z)
{
    this.type = System.Math.Type.Vec3

    if (x && x.type == System.Math.Type.Vec4)
    {
        this.x = x.x? x.x:0;
        this.y = x.y? x.y:0;
        this.z = x.z? x.z:0;
    }
    else
    {
        this.x = x? x:0;
        this.y = y? y:0;
        this.z = z? z:0;
    }

    this.Print = function()
    {
        System.Print("[", this.x, this.y, this.z, "]");
    }
}

Vec4 = function(x, y, z, w)
{
    this.type = System.Math.Type.Vec4

    if (x && x.type == System.Math.Type.Vec4)
    {
        this.x = x.x? x.x:0;
        this.y = x.y? x.y:0;
        this.z = x.z? x.z:0;
        this.w = x.w? x.w:0;
    }
    else if (x && x.type == System.Math.Type.Vec3)
    {
        this.x = x.x? x.x:0;
        this.y = x.y? x.y:0;
        this.z = x.z? x.z:0;
        this.w =   y?   y:0;
    }
    else
    {
        this.x = x? x:0;
        this.y = y? y:0;
        this.z = z? z:0;
        this.w = w? w:0;
    }

    this.Print = function()
    {
        System.Print("[", this.x, this.y, this.z, this.w, "]");
    }
}

Mat3 = function(c0, c1, c2)
{
    this.type = System.Math.Type.Mat3
    this.c0 = c0? c0:new vec3();
    this.c1 = c1? c1:new vec3();
    this.c2 = c2? c2:new vec3();

    this.Print = function()
    {
        this.c0.Print();
        this.c1.Print();
        this.c2.Print();
    }
}

Mat4 = function(c0, c1, c2, c3)
{
    this.type = System.Math.Type.Mat4
    this.c0 = c0? c0:new vec4();
    this.c1 = c1? c1:new vec4();
    this.c2 = c2? c2:new vec4();
    this.c3 = c3? c3:new vec4();

    this.Print = function()
    {
        this.c0.Print();
        this.c1.Print();
        this.c2.Print();
        this.c3.Print();
    }
}