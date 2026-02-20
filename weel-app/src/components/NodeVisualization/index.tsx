import React from 'react';
import { Canvas } from '@react-three/fiber';
import { OrbitControls } from '@react-three/drei';
import NodeGraph from './NodeGraph';

interface NodeVisualizationProps {
    width?: string;
    height?: string;
}

const NodeVisualization: React.FC<NodeVisualizationProps> = ({
    width = '100%',
    height = '100%'
}) => {
    return (
        <div style={{ width, height, background: '#222' }}>
            <Canvas camera={{ position: [0, 0, 20], fov: 50 }}>
                <color attach="background" args={['#222']} />

                <ambientLight intensity={0.4} />
                <directionalLight position={[10, 10, 10]} intensity={0.6} />
                <directionalLight position={[-10, -10, -10]} intensity={0.2} />

                <NodeGraph />

                <OrbitControls enablePan={true} enableZoom={true} enableRotate={true} />
            </Canvas>
        </div>
    );
};

export default NodeVisualization;