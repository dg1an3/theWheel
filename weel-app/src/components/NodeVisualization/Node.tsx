import React, { useRef, useMemo } from 'react';
import { useFrame } from '@react-three/fiber';
import { Html } from '@react-three/drei';
import { Elliptangle } from '../../geometry';
import { BORDER_RADIUS, computeEllipticalness, calcRectForActivation, createPlaqueGeometry } from '../../utils/geometryUtils';
import { ThreeEvent } from '@react-three/fiber';
import * as THREE from 'three';

interface NodeProps {
    id: string;
    position: [number, number, number];
    activation: number;
    color: string;
    highlight?: boolean;
    onClick?: (event: ThreeEvent<MouseEvent>, id: string) => void;
}

const Node: React.FC<NodeProps> = ({
    id,
    position,
    activation,
    color,
    highlight = false,
    onClick
}) => {
    const meshRef = useRef<THREE.Mesh>(null);
    const fanRef = useRef<THREE.Mesh>(null);

    // Create geometry for the node
    const geometry = useMemo(() => {
        // Calculate inner rect
        const rectInner = calcRectForActivation(activation);

        // Create elliptangle
        const ellipt = computeEllipticalness(activation);
        const elliptangle = new Elliptangle(rectInner, ellipt);

        // Calculate border radius
        let borderRadius = BORDER_RADIUS;
        if (rectInner.getSize(1) < borderRadius * 4.0) {
            borderRadius = rectInner.getSize(1) / 4.0;
        }

        // Create plaque geometry
        return createPlaqueGeometry(elliptangle, borderRadius, 8);
    }, [activation]);

    // Calculate scale factors
    const scale = useMemo(() => {
        const sizeUp = 100.0 + 1200.0 * (activation) * (activation);
        // We use the same scale for all dimensions to maintain proportions
        return sizeUp * 0.1;
    }, [activation]);

    // Rotate the node when highlighted
    useFrame((state, delta) => {
        if (highlight && meshRef.current && fanRef.current) {
            meshRef.current.rotation.z += delta * 0.5;
            fanRef.current.rotation.z += delta * 0.5;
        }
    });

    const handleClick = (event: ThreeEvent<MouseEvent>) => {
        if (onClick) {
            event.stopPropagation();
            onClick(event, id);
        }
    };

    return (
        <group position={position}>
            {/* Main geometry */}
            <mesh
                ref={meshRef}
                scale={[scale, scale, scale]}
                onClick={handleClick}
            >
                <sphereGeometry args={[1, 16, 16]} />
                <meshPhongMaterial
                    color={color}
                    specular={new THREE.Color(0x222222)}
                    shininess={30}
                />
            </mesh>

            <mesh
                ref={meshRef}
                scale={[scale, scale, scale]}
                onClick={handleClick}
            >
                <bufferGeometry>
                    <bufferAttribute
                        attach="attributes-position"
                        args={[new Float32Array(geometry.positions), 3]}
                    />
                    <bufferAttribute
                        attach="attributes-normal"
                        args={[new Float32Array(geometry.normals), 3]}
                    />
                    <bufferAttribute
                        attach="index"
                        args={[new Uint16Array(geometry.indices), 3]}
                    />
                </bufferGeometry>
                <meshPhongMaterial
                    color={color}
                    specular={new THREE.Color(0x222222)}
                    shininess={30}
                />
            </mesh>

            {/* Fan geometry */}
            <mesh
                ref={fanRef}
                scale={[scale, scale, scale]}
                onClick={handleClick}
            >
                <bufferGeometry>
                    <bufferAttribute
                        attach="attributes-position"
                        args={[new Float32Array(geometry.fanPositions), 3]}
                    />
                    <bufferAttribute
                        attach="attributes-normal"
                        args={[new Float32Array(geometry.fanNormals), 3]}
                    />

                    <bufferAttribute
                        attach="index"
                        args={[new Uint16Array(geometry.fanIndices), 3]}
                    />
                </bufferGeometry>
                <meshPhongMaterial
                    color={highlight ? "#ff4444" : "#994444"}
                    specular={new THREE.Color(0x222222)}
                    shininess={30}
                />
            </mesh>

            {/* Node label */}
            <Html
                position={[0, 0, 2 * scale]}
                style={{
                    color: 'white',
                    background: 'rgba(0,0,0,0.5)',
                    padding: '2px 5px',
                    borderRadius: '3px',
                    pointerEvents: 'none'
                }}
            >
                {id}: {activation.toFixed(2)}
            </Html>
        </group>
    );
};

export default Node;