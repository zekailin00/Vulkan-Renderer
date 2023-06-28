Vec3 = function(x, y, z)
{
    this.x = x? x:0;
    this.y = y? y:0;

    this.print = function()
    {
        system.print("[", this.x, this.y, "]");
    }
}

Vec3 = function(x, y, z)
{
    this.x = x? x:0;
    this.y = y? y:0;
    this.z = z? z:0;

    this.print = function()
    {
        system.print("[", this.x, this.y, this.z, "]");
    }
}

Vec4 = function(x, y, z, w)
{
    this.x = x? x:0;
    this.y = y? y:0;
    this.z = z? z:0;
    this.w = w? w:0;

    this.print = function()
    {
        system.print("[", this.x, this.y, this.z, this.w, "]");
    }
}

Mat3 = function(c0, c1, c2)
{
    this.c0 = c0? c0:new vec3();
    this.c1 = c1? c1:new vec3();
    this.c2 = c2? c2:new vec3();

    this.print = function()
    {
        this.c0.print();
        this.c1.print();
        this.c2.print();
    }
}

Mat4 = function(c0, c1, c2, c3)
{
    this.c0 = c0? c0:new vec4();
    this.c1 = c1? c1:new vec4();
    this.c2 = c2? c2:new vec4();
    this.c3 = c3? c3:new vec4();

    this.print = function()
    {
        this.c0.print();
        this.c1.print();
        this.c2.print();
        this.c3.print();
    }
}