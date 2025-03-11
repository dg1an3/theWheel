import { Extent2D } from './Extent2D';
import { Vector2D } from './Vector2D';

export abstract class RadialShape {
    outerRect: Extent2D;
    innerRect: Extent2D;
    discontinuities: number[];

    constructor() {
        this.outerRect = new Extent2D();
        this.innerRect = new Extent2D();
        this.discontinuities = [];
    }

    abstract eval(angle: number): Vector2D;

    abstract invEvalY(y: number): { angle1: number, angle2: number };
}