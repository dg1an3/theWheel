import React, { useState } from 'react';
import { useFrame } from '@react-three/fiber';
import { Html } from '@react-three/drei';
import Node from './Node';
import { ThreeEvent } from '@react-three/fiber';
import { NodeData } from '../../types';

const NodeGraph: React.FC = () => {
    const [nodes, setNodes] = useState<NodeData[]>([
        { id: "Node 1", position: [-5, 0, 0], activation: 0.2, color: "#8888ff", isSelected: false },
        { id: "Node 2", position: [0, 0, 0], activation: 0.5, color: "#88ff88", isSelected: false },
        { id: "Node 3", position: [5, 0, 0], activation: 0.8, color: "#ff8888", isSelected: false }
    ]);

    const [selectedNodeId, setSelectedNodeId] = useState<string | null>(null);
    const [autoRotate, setAutoRotate] = useState<boolean>(true);

    // Auto-animation for demonstration
    useFrame((state) => {
        if (autoRotate) {
            const time = state.clock.getElapsedTime();

            // Update node positions in a circular pattern
            setNodes(prev => prev.map((node, i) => {
                const angle = time * 0.2 + (i * Math.PI * 2) / prev.length;
                const radius = 5;

                return {
                    ...node,
                    position: [
                        Math.cos(angle) * radius,
                        Math.sin(angle) * radius,
                        0
                    ]
                };
            }));
        }
    });

    const handleNodeClick = (_event: ThreeEvent<MouseEvent>, nodeId: string) => {
        setSelectedNodeId(prev => prev === nodeId ? null : nodeId);

        setNodes(prev => prev.map(node => ({
            ...node,
            isSelected: node.id === nodeId ? !node.isSelected : false
        })));
    };

    const handleActivationChange = (nodeId: string, value: number) => {
        setNodes(prev => prev.map(node =>
            node.id === nodeId ? { ...node, activation: value } : node
        ));
    };

    return (
        <>
            {nodes.map((node) => (
                <Node
                    key={node.id}
                    id={node.id}
                    position={node.position}
                    activation={node.activation}
                    color={node.color}
                    highlight={node.isSelected}
                    onClick={handleNodeClick}
                />
            ))}

            <Html position={[0, -8, 0]} style={{ width: '300px' }}>
                <div className="control-panel">
                    <h3 style={{ color: 'white', margin: '0 0 10px 0' }}>Node Controls</h3>
                    {nodes.map((node) => (
                        <div key={node.id} style={{ margin: '10px 0' }}>
                            <div style={{ display: 'flex', alignItems: 'center' }}>
                                <div style={{ width: '80px', color: 'white' }}>{node.id}:</div>
                                <input
                                    type="range"
                                    min="0.01"
                                    max="1"
                                    step="0.01"
                                    value={node.activation}
                                    onChange={(e) => {
                                        handleActivationChange(node.id, parseFloat(e.target.value));
                                    }}
                                    style={{ flex: 1 }}
                                />
                                <div style={{ width: '40px', color: 'white', textAlign: 'right' }}>
                                    {node.activation.toFixed(2)}
                                </div>
                            </div>
                            <button
                                onClick={() => handleNodeClick({} as ThreeEvent<MouseEvent>, node.id)}
                                style={{
                                    background: node.isSelected ? '#ff4444' : '#444444',
                                    color: 'white',
                                    border: 'none',
                                    padding: '5px 10px',
                                    marginTop: '5px',
                                    cursor: 'pointer',
                                    borderRadius: '3px'
                                }}
                            >
                                {node.isSelected ? 'Deselect' : 'Select'}
                            </button>
                        </div>
                    ))}
                    <div style={{ marginTop: '20px' }}>
                        <label style={{ color: 'white', display: 'flex', alignItems: 'center' }}>
                            <input
                                type="checkbox"
                                checked={autoRotate}
                                onChange={() => setAutoRotate(!autoRotate)}
                                style={{ marginRight: '10px' }}
                            />
                            Auto-rotate nodes
                        </label>
                    </div>
                </div>
            </Html>
        </>
    );
};

export default NodeGraph;