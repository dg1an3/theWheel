import { Extent2D } from '../components/NodeVisualization/geometry/Extent2D';
import { Vector2D } from '../components/NodeVisualization/geometry/Vector2D';
import { RadialShape } from '../components/NodeVisualization/geometry/RadialShape';
import { PlaqueGeometry } from '../types';

// Constants
export const BORDER_RADIUS = 2.0;
export const RECT_SIZE_MAX = 13.0 / 16.0;
export const RECT_SIZE_SCALE = 6.0 / 16.0;

// Compute ellipticalness based on activation value
export const computeEllipticalness = (activation: number): number => {
    const scale = 1.0 - 1.0 / Math.sqrt(2.0);
    return 1.0 - scale * Math.exp(-1.5 * activation + 0.01);
};

// Calculate rect for activation
export const calcRectForActivation = (activation: number): Extent2D => {
    // Compute the desired aspect ratio
    const aspectRatio = RECT_SIZE_MAX - RECT_SIZE_SCALE * Math.exp(-2.0 * activation);

    // Compute the new width and height
    const sx = Math.sqrt(activation / aspectRatio) / 1.0;
    const sy = Math.sqrt(activation * aspectRatio) / 1.0;

    // Create and update extent
    const extent = new Extent2D();
    extent.deflate(-sx, -sy, -sx, -sy);

    return extent;
};

// Helper to calculate vertices for a section
export const calcVertsForSection = (
    theta: number,
    offset: Vector2D,
    positions: number[],
    normals: number[],
    borderRadius: number,
    numSteps: number
): void => {
    // Step from angle = 0 to PI/2
    const step = (Math.PI / 2.0) / (numSteps - 1.0);

    // Create vertices along the curve
    for (let i = 0; i < numSteps; i++) {
        const angle = i * step;

        // Position
        const x = Math.cos(angle);
        const y = 0;
        const z = Math.sin(angle);

        // Create normal
        const nx = Math.cos(theta) * x;
        const ny = Math.sin(theta) * x;
        const nz = z;

        // Transform point
        const px = Math.cos(theta) * x * borderRadius + offset.x;
        const py = Math.sin(theta) * x * borderRadius + offset.y;
        const pz = z * borderRadius;

        // Add vertex
        positions.push(px, py, pz);
        normals.push(nx, ny, nz);
    }

    // Add center point for each section
    positions.push(offset.x, offset.y, borderRadius);
    normals.push(0, 0, 1);
};

// Create geometry for plaque
export const createPlaqueGeometry = (
    shape: RadialShape,
    borderRadius: number,
    numSteps: number
): PlaqueGeometry => {
    const positions: number[] = [];
    const normals: number[] = [];
    const indices: number[] = [];

    // Track discontinuities
    const discontinuities = shape.discontinuities;
    let nextDiscont = 0;

    // Step size for the four quadrants
    const step = 2.0 * Math.PI / (numSteps * 4.0);

    // Store section starts for indexing
    const sectionStarts: number[] = [];

    // Step through the angles
    let theta = 0.0;
    for (let slice = 0; slice < numSteps * 4; slice++) {
        // Are we at the last step?
        if (slice + 1 === numSteps * 4) {
            // Make sure we close where we started
            theta = 0.0;
        }

        // Evaluate the next shape point
        const pt = shape.eval(theta);

        // Add index of next section
        sectionStarts.push(positions.length / 3);

        // Calculate vertices for new section
        calcVertsForSection(theta, pt, positions, normals, borderRadius, numSteps);

        // Next angle
        theta += step;

        // Have we passed the next discontinuity?
        if (nextDiscont < discontinuities.length && theta > discontinuities[nextDiscont]) {
            theta = discontinuities[nextDiscont++];

            // Add an extra slice for the discontinuity
            slice--;
        }
    }

    // Create indices for triangle strips between sections
    for (let i = 0; i < sectionStarts.length - 1; i++) {
        const current = sectionStarts[i];
        const next = sectionStarts[i + 1];

        for (let j = 0; j < numSteps - 1; j++) {
            // Create two triangles for each quad
            indices.push(current + j, next + j, current + j + 1);
            indices.push(current + j + 1, next + j, next + j + 1);
        }

        // Connect to center point
        indices.push(current + numSteps - 1, next + numSteps - 1, current + numSteps);
        indices.push(current + numSteps, next + numSteps - 1, next + numSteps);
    }

    // Create fan geometry
    const fanPositions: number[] = [];
    const fanNormals: number[] = [];
    const fanIndices: number[] = [];

    // Add center point
    const Y = shape.innerRect.getMin().y + (shape.innerRect.getSize(1)) * 0.75;
    fanPositions.push(0, Y, borderRadius + 0.1);
    fanNormals.push(0, 0, 1);

    // Calculate angles
    let angleStart = 0.0, angleEnd = Math.PI;
    const angles = shape.invEvalY(Y);
    angleStart = angles.angle1;
    angleEnd = angles.angle2;

    const angleDiff = angleEnd - angleStart;
    for (let i = 0; i <= numSteps * 4; i++) {
        const currentAngle = angleStart + angleDiff * (i / (numSteps * 4));
        const pt = shape.eval(currentAngle);

        fanPositions.push(pt.x, pt.y, borderRadius - 0.1);
        fanNormals.push(0, 0, 1);

        // Create triangle fan indices
        if (i > 0) {
            fanIndices.push(0, i, i - 1);
        }
    }

    return {
        positions,
        normals,
        indices,
        fanPositions,
        fanNormals,
        fanIndices
    };
};