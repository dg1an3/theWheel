import React, { useState, useMemo, useCallback } from 'react';
import ElliptangleCanvas from './ElliptangleCanvas';
import ElliptangleControls from './ElliptangleControls';
import { ElliptangleOptions } from '../../geometry';
import './ElliptangleEditor.css';

const ElliptangleEditor: React.FC = () => {
    const [activation, setActivation] = useState(80);
    const [curve, setCurve] = useState(25);
    const [shapeWidth, setShapeWidth] = useState(280);
    const [shapeHeight, setShapeHeight] = useState(200);
    const [options, setOptions] = useState<ElliptangleOptions>({
        fillColor: '#4CAF50',
        headerFillColor: '#81C784',
        topLeftStroke: '#81C784',
        bottomRightStroke: '#2E7D32',
        strokeWidth: 3,
        text: 'Hello',
        textColor: '#FFFFFF',
        fontFamily: 'Arial, sans-serif',
        fontSizeRatio: 0.3
    });

    const adjustActivation = useCallback((value: number) => {
        setActivation(value);
        setCurve(Math.round(2000.0 / Math.max(value, 1)));
        setShapeWidth(Math.max(100, Math.round(value * 3.5)));
        setShapeHeight(Math.max(80, Math.round(value * 2.5)));
    }, []);

    const handleOptionsChange = useCallback((partial: Partial<ElliptangleOptions>) => {
        setOptions(prev => ({ ...prev, ...partial }));
    }, []);

    const memoizedOptions = useMemo(() => ({ ...options }), [options]);

    return (
        <div className="elliptangle-editor">
            <ElliptangleCanvas
                width={600}
                height={400}
                curve={curve}
                shapeWidth={shapeWidth}
                shapeHeight={shapeHeight}
                options={memoizedOptions}
            />
            <ElliptangleControls
                activation={activation}
                curve={curve}
                shapeWidth={shapeWidth}
                shapeHeight={shapeHeight}
                options={options}
                onActivationChange={adjustActivation}
                onCurveChange={setCurve}
                onWidthChange={setShapeWidth}
                onHeightChange={setShapeHeight}
                onOptionsChange={handleOptionsChange}
            />
        </div>
    );
};

export default ElliptangleEditor;
