export class Vector2D {
    x: number;
    y: number;

    constructor(x = 0, y = 0) {
        this.x = x;
        this.y = y;
    }

    clone(): Vector2D {
        return new Vector2D(this.x, this.y);
    }
}