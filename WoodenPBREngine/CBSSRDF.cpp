#include "pch.h"
#include "CBSSRDF.h"

WPBR_BEGIN

DECL_OUT_COMP_DATA(CTabulatedBSSRDF)
DECL_OUT_COMP_DATA(CBSSRDFTable)
DECL_OUT_COMP_DATA(CBSSRDFTableEntity)
HEntity STabulatedBSSRDF::create(const Spectrum& absorb, const Spectrum& scatter, float eta, HEntity hTable)
{
	Spectrum transmitance = absorb + scatter;
	Spectrum transmittanceNoZero = cndLoad(cmpEQL(transmitance, 0.0f), 1.0f, transmitance);
	Spectrum rho = scatter / transmittanceNoZero;

	MEngine& engine = MEngine::getInstance();
	HEntity hEntity = engine.createEntity();


	CBSSRDFTableEntity table; table.h = hEntity;
	engine.addComponent<CBSSRDFTableEntity>(hEntity, table);

	CTabulatedBSSRDF tabulatedBSSRDF;
	tabulatedBSSRDF.tr = transmitance;
	tabulatedBSSRDF.rho = rho;
	engine.addComponent<CTabulatedBSSRDF>(hEntity, tabulatedBSSRDF);

	return hEntity;
}

Spectrum STabulatedBSSRDF::sr(const CTabulatedBSSRDF& bssrdf, const CBSSRDFTable& table, float r)
{
	Spectrum s(0.0f);
	for (int lambda = 0; lambda < nSpectralSamples; lambda++)
	{
		float rOptical = r * bssrdf.tr[lambda];
		int rhoOffset, radiusOffset;
		float rhoWeights[4], radiusWeights[4];
		if (!catmullRomWeights(table.nRhoSamples, table.rhoSamples.data(),
			bssrdf.rho[lambda], &rhoOffset, rhoWeights) ||
			!catmullRomWeights(table.nRadiusSamples, table.radiusSamples.data(),
			rOptical, &radiusOffset, radiusWeights))
			continue;

		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				float weight = rhoWeights[i] * radiusWeights[j];
				uint32_t iRho = rhoOffset + i;
				uint32_t iRadius = radiusOffset + j;

				if (weight != 0.0f)
				{
					s[lambda] += weight*table.eval(iRho, iRadius);
				}
			}
		}

		if (rOptical != 0.0f)
		{
			s[lambda] /= 2 * PI*rOptical;
		}
	}

	s *= bssrdf.tr*bssrdf.tr;
	return s;
}

WPBR_END