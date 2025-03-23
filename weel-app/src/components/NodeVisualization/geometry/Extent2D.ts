import { Vector2D } from './Vector2D';

export class Extent2D {
    min: Vector2D;
    max: Vector2D;

    constructor(minX = -1, minY = -1, maxX = 1, maxY = 1) {
        this.min = new Vector2D(minX, minY);
        this.max = new Vector2D(maxX, maxY);
    }

    getSize(dimension: number): number {
        return dimension === 0 ? this.max.x - this.min.x : this.max.y - this.min.y;
    }

    getMin(): Vector2D {
        return this.min.clone();
    }

    getMax(): Vector2D {
        return this.max.clone();
    }

    setMin(v: Vector2D): void {
        this.min.x = v.x;
        this.min.y = v.y;
    }

    setMax(v: Vector2D): void {
        this.max.x = v.x;
        this.max.y = v.y;
    }

    clone(): Extent2D {
        return new Extent2D(this.min.x, this.min.y, this.max.x, this.max.y);
    }

    deflate(dx1: number, dy1: number, dx2: number, dy2: number): Extent2D {
        this.min.x -= dx1;
        this.min.y -= dy1;
        this.max.x += dx2;
        this.max.y += dy2;
        return this;
    }
}