import { RadialShape } from './RadialShape';
import { Extent2D } from './Extent2D';
import { Vector2D } from './Vector2D';

export class Elliptangle extends RadialShape {
    fatEllipseExtent: Extent2D;
    tallEllipseExtent: Extent2D;

    constructor(rectInner: Extent2D, ellipt: number) {
        super();
        this.fatEllipseExtent = new Extent2D();
        this.tallEllipseExtent = new Extent2D();
        this.setExtentEllipt(rectInner, ellipt);
    }

    setExtentEllipt(rectInner: Extent2D, ellipt: number): void {
        // Set inner extent
        this.innerRect = rectInner.clone();

        // Calculate outer extent
        const outerRect = rectInner.clone();
        outerRect.deflate(
            -(1.0 - ellipt) * rectInner.getSize(0) / 2.0,
            -(1.0 - ellipt) * rectInner.getSize(1) / 2.0,
            -(1.0 - ellipt) * rectInner.getSize(0) / 2.0,
            -(1.0 - ellipt) * rectInner.getSize(1) / 2.0
        );
        this.outerRect = outerRect;

        // Calculate the fat ellipse extent
        const ry = this.innerRect.getSize(1) / this.outerRect.getSize(1);
        const by = Math.sqrt(this.innerRect.getSize(0) * this.innerRect.getSize(0) * 0.25 / (1.0 - ry * ry));

        this.fatEllipseExtent = this.outerRect.clone();
        this.fatEllipseExtent.deflate(
            this.fatEllipseExtent.getSize(0) / 2.0, 0,
            this.fatEllipseExtent.getSize(0) / 2.0, 0
        );
        this.fatEllipseExtent.deflate(-by, 0, -by, 0);

        // Calculate the tall ellipse extent
        const rx = this.innerRect.getSize(0) / this.outerRect.getSize(0);
        const bx = Math.sqrt(this.innerRect.getSize(1) * this.innerRect.getSize(1) * 0.25 / (1.0 - rx * rx));

        this.tallEllipseExtent = this.outerRect.clone();
        this.tallEllipseExtent.deflate(
            0, this.tallEllipseExtent.getSize(1) / 2.0,
            0, this.tallEllipseExtent.getSize(1) / 2.0
        );
        this.tallEllipseExtent.deflate(0, -bx, 0, -bx);

        // Set up discontinuity list
        const sectionAngle = Math.atan2(this.outerRect.getSize(1), this.outerRect.getSize(0));
        this.discontinuities = [
            sectionAngle,
            Math.PI - sectionAngle,
            Math.PI + sectionAngle,
            2.0 * Math.PI - sectionAngle
        ];
    }

    eval(theta: number): Vector2D {
        let point = new Vector2D();

        // Check theta range
        if (theta < 0.0) {
            return this.eval(theta + 2.0 * Math.PI);
        } else if (2.0 * Math.PI < theta) {
            return this.eval(theta - 2.0 * Math.PI);
        } else {
            // Compare tan(theta) to slope to see which section we're in
            const slope = this.outerRect.getSize(1) / this.outerRect.getSize(0);

            let a, b;
            if (Math.abs(Math.tan(theta)) >= Math.abs(slope)) {
                // Tangent greater than slope, so we're in fat ellipse domain
                a = this.fatEllipseExtent.getSize(0) / 2.0;
                b = this.fatEllipseExtent.getSize(1) / 2.0;
            } else {
                // Tangent less than slope, so we're in tall ellipse domain
                a = this.tallEllipseExtent.getSize(0) / 2.0;
                b = this.tallEllipseExtent.getSize(1) / 2.0;
            }

            // Compute theta_prime for the ellipse
            const sign = Math.cos(theta) > 0 ? 1.0 : -1.0;
            const theta_prime = Math.atan2(sign * a * Math.tan(theta), sign * b);

            point.x = a * Math.cos(theta_prime);
            point.y = b * Math.sin(theta_prime);
        }

        return point;
    }

    invEvalY(y: number): { angle1: number, angle2: number } {
        // Compute point on tall ellipse
        let a = this.tallEllipseExtent.getSize(0) / 2.0;
        let b = this.tallEllipseExtent.getSize(1) / 2.0;
        let x = a * Math.sqrt(1.0 - y * y / (b * b));

        // See if the point is greater than tall slope
        const slope = this.outerRect.getSize(1) / this.outerRect.getSize(0);
        if (Math.abs(y / x) >= Math.abs(slope)) {
            // Yes, so compute for fat ellipse
            a = this.fatEllipseExtent.getSize(0) / 2.0;
            b = this.fatEllipseExtent.getSize(1) / 2.0;
            x = a * Math.sqrt(1.0 - y * y / (b * b));
        }

        // Return the angles
        return {
            angle1: Math.atan2(y, x),
            angle2: Math.atan2(y, -x)
        };
    }
}