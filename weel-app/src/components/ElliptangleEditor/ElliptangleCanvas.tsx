import React, { useRef, useEffect } from 'react';
import { ElliptangleRenderer, ElliptangleOptions } from '../../geometry';

interface ElliptangleCanvasProps {
    width: number;
    height: number;
    curve: number;
    shapeWidth: number;
    shapeHeight: number;
    options: ElliptangleOptions;
}

const ElliptangleCanvas: React.FC<ElliptangleCanvasProps> = ({
    width,
    height,
    curve,
    shapeWidth,
    shapeHeight,
    options
}) => {
    const canvasRef = useRef<HTMLCanvasElement>(null);
    const rendererRef = useRef<ElliptangleRenderer | null>(null);

    useEffect(() => {
        if (canvasRef.current && !rendererRef.current) {
            rendererRef.current = new ElliptangleRenderer(canvasRef.current);
        }
    }, []);

    useEffect(() => {
        if (!rendererRef.current || !canvasRef.current) return;

        const renderer = rendererRef.current;
        renderer.clear();

        const x = (canvasRef.current.width - shapeWidth) / 2;
        const y = (canvasRef.current.height - shapeHeight) / 2;

        renderer.drawElliptangleWithSeparateStrokes(x, y, shapeWidth, shapeHeight, curve, options);
    }, [curve, shapeWidth, shapeHeight, options]);

    return (
        <canvas
            ref={canvasRef}
            width={width}
            height={height}
            style={{
                border: '2px solid #333',
                background: 'gray',
                borderRadius: '8px',
                boxShadow: '0 4px 8px rgba(0,0,0,0.1)',
                maxWidth: '100%'
            }}
        />
    );
};

export default ElliptangleCanvas;
