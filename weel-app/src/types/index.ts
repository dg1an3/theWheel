// Common type definitions
export type Position3D = [number, number, number];

export interface NodeData {
    id: string;
    position: Position3D;
    activation: number;
    color: string;
    isSelected?: boolean;
}

export interface PlaqueGeometry {
    positions: number[];
    normals: number[];
    indices: number[];
    fanPositions: number[];
    fanNormals: number[];
    fanIndices: number[];
}